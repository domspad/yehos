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

#define STACK_PTABLE_ADDR 0xffffe000
#define SWAP_ADDR 0xffbfe000

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
        uint32_t lba = (ptable_entry & ~(uint32_t) 0xf0000000) >> 12;
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
        ptable[page_table_index] = DISK_MEMORY_ADDR_FLAG + (lba << 12); // 0x4[lba]000
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
        ptable[(virt_addr >> 12) + lba/2] = DISK_MEMORY_ADDR_FLAG + ((first_lba + lba) << 12); //0x4[lba]000
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
copy_to_new_physical_page(virtaddr_t source) {
    physaddr_t new_physaddr = get_unused_page();

    // SWAP_ADDR is the virtual address we reserve for copying to arbitrary
    // physical memory.
    set_ptable_entry(SWAP_ADDR, new_physaddr);
    asm volatile ( "invlpg (%0)" : : "b"(SWAP_ADDR) : "memory" );
    memcpy((void *) SWAP_ADDR, (void *) source, 4096);
    return new_physaddr;
}

physaddr_t
clone_page_directory()
{
    virtaddr_t *current_pagedir = (void *) 0xfffff000;

    // Copy the stack and the page table that refers to it.
    physaddr_t new_phys_stack = copy_to_new_physical_page(VIRTUAL_STACK_ADDR);
    physaddr_t new_phys_stack_ptable = copy_to_new_physical_page(STACK_PTABLE_ADDR);

    physaddr_t new_cr3 = copy_to_new_physical_page((virtaddr_t) current_pagedir);
    // The new page physical page directory is mapped to the swap page out of
    // copy_to_new_physical_page. We can use that fact to configure the new pagedir.
    virtaddr_t *new_pagedir = (void *) SWAP_ADDR;

    // Mark pagetables and the pages they refer to as COW starting from the
    // first non-id mapped page up to but not including the pagetable that
    // refers to the stack and the page directory itself.
    for (int i = 1; i < ENTRIES_PER_PAGE-2; ++i) {
        if (page_is_present(current_pagedir[i])) {
            // We multiply the index into the page directory by the number of
            // entries per page to get an index into the page table.
            make_ptable_entries_cow((ptable_index_t) i*ENTRIES_PER_PAGE);
            current_pagedir[i] = new_pagedir[i] = make_cow(current_pagedir[i]);
        }
    }

    // The new physical stack becomes the real stack for the current address space.
    set_ptable_entry(STACK_PTABLE_ADDR, new_phys_stack_ptable);
    set_ptable_entry(VIRTUAL_STACK_ADDR, new_phys_stack);

    // We're done with the swap page for now - don't point it to any physical memory.
    set_ptable_entry(SWAP_ADDR, 0x00000000);

    new_pagedir[1023] = make_present_and_rw(new_cr3);
    return new_cr3;
}

