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
#define PTABLE_ADDR 0xffc00000
#define DISK_MEMORY_ADDR_FLAG 0x40000000

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
    uint32_t *source = (void *) BASE_OF_KERNEL_STACK;
    uint32_t *dest = (void *) BASE_OF_VIRTUAL_STACK;
    memcpy(dest, source, STACK_SIZE);

    // asm call to move esp up to the virtual stack
    uint32_t distance = DISTANCE_TO_VIRTUAL_STACK;
    asm volatile  ("add %0, %%esp" : : "b" (distance));

}

physaddr_t
get_unused_page()
{
    physaddr_t ret = g_nextPage;
    g_nextPage += 0x1000;
    return ret;
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

pagetable_entry_t
make_cow(pagetable_entry_t entry)
{
    entry &= ~READ_WRITE;
    entry |= COPY_ON_WRITE;
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

int
test_cow()
{
    int volatile *b = (int *) 0x000000;
    *b = *b + 1;

    pagetable_entry_t *a = &ptable[((uint32_t) b >> 12)];
    *a = make_cow(*a);

    asm volatile ( "invlpg (%0)" : : "b"(b) : "memory" );

    b[0] = b[0] + 1;

    return 0;
}
