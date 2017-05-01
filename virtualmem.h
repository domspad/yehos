#ifndef VIRTUALMEM_H
#define VIRTUALMEM_H
#define ISO_START 0x100000
#include "ata.h"

void handle_page_fault(void);
void setup_paging(void);
void mmap_disk(ata_disk *d);
int mmap(char *filename, uint32_t virt_addr);
#endif
