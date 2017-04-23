#include "kernel.h"
#include "asmhelpers.h"
#include "ata.h"

#define PAGEDIR_ADDR 0x80000 //random spot, could have been anywhere
#define PT0_ADDR 0x81000    // putting the first page table after the page directory
#define PRESENT_AND_RW 0x03
#define PTABLE_ADDR 0xffc00000
#define DISK_MEMORY_ADDR_FLAG 0x40000000

physaddr_t g_nextPage = 0x100000;
uint32_t *ptable = (uint32_t *) 0xffc00000;

void
setup_paging()
{
    // setting up the page directory and the first page table
    uint32_t *pagedir = (void *) PAGEDIR_ADDR;
    memset(pagedir, 0, 4096);
    pagedir[0] = PT0_ADDR | PRESENT_AND_RW;
    pagedir[1023] = PAGEDIR_ADDR | PRESENT_AND_RW;

    uint32_t *pt0 = (void *) PT0_ADDR;
    memset(pt0, 0, 4096);

    // identity-map first 1MB
    // set the first 256 entries of page table 0
    for (unsigned int i=0; i<256; ++i) {
        pt0[i] = (i << 12) | PRESENT_AND_RW;
    }

    set_cr3(PAGEDIR_ADDR);

    set_cr0_paging();
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

    physaddr_t page = get_unused_page();
    ptable[pfaddr >> 12] = page | PRESENT_AND_RW;

    if ((ptable_entry >> 28) == 4) {
        // "Use the fours"
        uint32_t lba = (ptable_entry & ~(uint32_t) 0xF0000000) >> 4;
        ata_disk *d = &disks[0];
        char *diskbuf = (void *) 0x90000;
        while (atapi_read_lba(d, diskbuf, 0xFFFF, lba, 2) < 0) {
            kprintf("timeout on sector %d\n", lba);
        }
        memcpy((void *) (pfaddr & 0xfffff000), diskbuf, d->sector_size*2);
    }
}

void
mmap_disk(ata_disk *d) {

    // telling the MMU that the iso can be found starting at the first MB
    // of virtual memory, and that we have "loaded" all of it in.
    uint32_t iso_page_start = 0x100;
    uint32_t page_size = 0x1000;
    uint32_t npages = d->max_lba * d->sector_size / page_size + 1;

    for (uint32_t i = 0; i < npages; i++) {
        uint32_t lba = i * page_size / d->sector_size;
        ptable[i + iso_page_start] = DISK_MEMORY_ADDR_FLAG + (lba << 4); // 0x4[lba]0
    }
}
