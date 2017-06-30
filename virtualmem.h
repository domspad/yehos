#ifndef VIRTUALMEM_H
#define VIRTUALMEM_H
#define ISO_START 0x100000
#define IDENTITY_MAP_END 0x40000000
#define SWAP_STACK 0xffbfe000
#include "ata.h"

void handle_page_fault(void);
void setup_paging(void);
void setup_virtual_stack(void);
physaddr_t get_unused_page(void);
void mmap_disk(ata_disk *d);
int mmap(char *filename, uint32_t virt_addr);

pagetable_entry_t make_present_and_rw(pagetable_entry_t entry);
physaddr_t clone_page_directory(void);
#endif

