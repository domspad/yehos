#include "kernel.h"
#include "asmhelpers.h"
#include "ata.h"
#include "iso9660.h"
#include "memlib.h"
#include "virtualmem.h"

#define PAGEDIR_ADDR 0x80000
#define PT0_ADDR 0x81000
#define READ_WRITE 0x2
#define PRESENT_AND_RW 0x03
#define COPY_ON_WRITE 0x200
#define DISK_MEMORY_ADDR_FLAG 0x40000000

#define ENTRIES_PER_PAGE 1024

#define KERNEL_STACK_ADDR 0x7f000
#define VIRTUAL_STACK_ADDR 0xffbff000
#define STACK_SIZE 0xfff

typedef uint32_t page[0xfff >> 2];

int swap_page_index = 0;
page swap_pages[2];

physaddr_t g_nextPage = 0x100000;
pagetable_entry_t *ptable = (pagetable_entry_t *) 0xffc00000;

void
setup_paging()
{
    uint32_t *pagedir = (void *) PAGEDIR_ADDR;
    memset(pagedir, 0, 4096);
    pagedir[0] = make_present_and_rw(PT0_ADDR);
    pagedir[1023] = make_present_and_rw(PAGEDIR_ADDR);

    uint32_t *pt0 = (void *) PT0_ADDR;
    memset(pt0, 0, 4096);

    // identity-map first 1MB
    for (unsigned int i=0; i<256; ++i) {
        pt0[i] = (i << 12) | PRESENT_AND_RW;
    }

    set_cr3(PAGEDIR_ADDR);

    set_cr0_paging();

}

void
setup_virtual_stack()
{
    uint32_t *source = (void *) KERNEL_STACK_ADDR;
    uint32_t *dest = (void *) VIRTUAL_STACK_ADDR;
    memcpy(dest, source, STACK_SIZE);

    // asm call to move esp up to the virtual stack
    uint32_t stack_ptr_change = VIRTUAL_STACK_ADDR - KERNEL_STACK_ADDR;
    asm volatile  ("add %0, %%esp" : : "b" (stack_ptr_change));

}

physaddr_t
get_unused_page()
{
    physaddr_t ret = g_nextPage;
    g_nextPage += 0x1000;
    return ret;
}

// Set a virtual address to point to a specific physical page
void
set_ptable_entry(virtaddr_t virt_addr, physaddr_t phys_addr) {
    ptable[virt_addr >> 12] = make_present_and_rw(phys_addr);
}

void
handle_page_fault()
{
    uint32_t pfaddr = get_cr2();
    uint32_t ptable_entry = ptable[pfaddr >> 12];

    if (is_cow(ptable_entry)) {
        memcpy(swap_pages[swap_page_index], (void *) (pfaddr & 0xfffff000), 4096);
        swap_page_index = (swap_page_index + 1) % 2;
    }

    physaddr_t page = get_unused_page();
    ptable[pfaddr >> 12] = make_present_and_rw(page);

    // test whether the ptable entry refers to the iso filesystem
    if (is_cow(ptable_entry)) {
        memcpy((void *) (pfaddr & 0xfffff000), swap_pages[swap_page_index], 4096);
    }
    else if ((ptable_entry >> 28) == 4) {
        // "Use the fours"
        uint32_t lba = (ptable_entry & ~(uint32_t) 0xf0000000) >> 4;
        ata_disk *d = &disks[0];
        char *diskbuf = (void *) 0x90000;
        while (atapi_read_lba(d, diskbuf, 0xffff, lba, 2) < 0) {
            kprintf("timeout on sector %d\n", lba);
        }
        memcpy((void *) (pfaddr & 0xfffff000), diskbuf, d->sector_size*2);
    }
}

void
mmap_disk(ata_disk *d) {
    uint32_t page_size = 0x1000;
    uint32_t npages = d->max_lba * d->sector_size / page_size + 1;
    
    for (uint32_t i = 0; i < npages; i++) {
        uint32_t lba = i * page_size / d->sector_size;
        uint32_t page_table_index = i + (ISO_START >> 12);
        ptable[page_table_index] = DISK_MEMORY_ADDR_FLAG + (lba << 4); // 0x4[lba]0
    }
}

const DirectoryRecord *find_file(const char* filename){
    
    const DirectoryRecord *entry = get_first_entry((void*)(uint32_t)(ISO_START) ); 
    while((entry->record_len > 0)){
       if( strncmp(filename, entry->id, strlen(filename)) == 0) {
            return entry;
       }
       entry = NEXT_DIR_ENTRY(entry);
    }
    return NULL;
}

int 
mmap(char *filename, uint32_t virt_addr){
    // using the mmaped iso, determine the first lba and size of file
    uint32_t first_lba;
    uint32_t num_lbas;
    const DirectoryRecord *entry = find_file(filename);
    if (entry == NULL){
        kprintf("can't find %s\n", filename);
        return -1;
    }
    first_lba = entry->data_sector;
    num_lbas = entry->data_len / 2048 + 1;
    kprintf("%s at %x for %d sectors\n", filename, first_lba, num_lbas);
    // mmap the file to the given virt addr using "the fours" scheme.
    // (assumes ratio of sector size to page size is 1 : 2
    for(uint32_t lba = 0; lba < num_lbas; lba += 2)
        ptable[(virt_addr >> 12) + lba/2] = DISK_MEMORY_ADDR_FLAG + ((first_lba + lba) << 4); //0x4[lba]0
    return 0;
}

// Returns 1 iff the present bit is set to 1 on a ptable entry.
int
page_is_present(pagetable_entry_t entry) {
  return entry & 1;
}

pagetable_entry_t
make_cow(pagetable_entry_t entry)
{
    if (page_is_present(entry)) {
        entry &= ~READ_WRITE;
        entry |= COPY_ON_WRITE;
    }
    return entry;
}

pagetable_entry_t
make_present_and_rw(pagetable_entry_t entry)
{
    entry |= PRESENT_AND_RW;
    return entry;
}

int
is_cow(pagetable_entry_t entry)
{
    // the COPY_ON_WRITE bit is the 10th bit
    return (entry >> 9) & 1; 
}

void
make_ptable_entries_cow(ptable_index_t start_index) {
    ptable_index_t stack_ptable_index = VIRTUAL_STACK_ADDR >> 12;

    for (int i = start_index; i < start_index + ENTRIES_PER_PAGE; i++) {
        if (i != stack_ptable_index) {
            ptable[i] = make_cow(ptable[i]);
        }
    }
}

physaddr_t
clone_page_directory()
{
    virtaddr_t *current_pagedir = (void *) 0xfffff000;

    // Insert our new pagedir into the current address space so we can write to it.
    physaddr_t new_cr3 = get_unused_page();
    virtaddr_t *new_pagedir = (void *) 0xffbfd000;
    set_ptable_entry((virtaddr_t) new_pagedir, new_cr3);

    // Copy stack to swap page.
    // We're assuming the stack only occupies a single page.
    physaddr_t new_phys_stack = get_unused_page();
    set_ptable_entry(SWAP_STACK, new_phys_stack);
    memcpy((void *) SWAP_STACK, (void *) VIRTUAL_STACK_ADDR, STACK_SIZE);

    // Copy the page table that references the stack to a swap page
    virtaddr_t *current_stack_ptable = (void *) 0xffffe000;
    virtaddr_t *swap_stack_ptable = (void *) 0xffbfc000;
    physaddr_t new_phys_stack_ptable = get_unused_page();
    set_ptable_entry((virtaddr_t) swap_stack_ptable, new_phys_stack_ptable);
    memcpy(swap_stack_ptable, current_stack_ptable, 4096);

    // Make cow from the first non-identity mapped page up to (but not including) the page directory itself.
    ptable_index_t first_nonident_ptable_idx = 1;
    // exclude the page directory itself and the page table that refers to the stack
    for (int i = 0; i < ENTRIES_PER_PAGE-1; ++i) {
        // Copy identity-mapped pages to new pages directory without COW.
        if (i < first_nonident_ptable_idx) {
            new_pagedir[i] = current_pagedir[i];
        } else if (i == 1022) {
            // ptable that refers to the stack
            new_pagedir[i] = current_pagedir[i]; 
        } else {
            if (page_is_present(current_pagedir[i])) {
                make_ptable_entries_cow((ptable_index_t) i*ENTRIES_PER_PAGE);
                current_pagedir[i] = new_pagedir[i] = make_cow(current_pagedir[i]);
            } else {
                current_pagedir[i] = new_pagedir[i];
            }
        }
    }

    // the physical swap stack becomes the real stack for the current address space
    virtaddr_t stack_ptable_entry = ptable[VIRTUAL_STACK_ADDR >> 12];
    set_ptable_entry((virtaddr_t) current_stack_ptable, new_phys_stack_ptable);
    set_ptable_entry(VIRTUAL_STACK_ADDR, new_phys_stack);
    ptable[SWAP_STACK >> 12] = 0x0000000;

    new_pagedir[1023] = make_present_and_rw(new_cr3);
    return new_cr3;
}

