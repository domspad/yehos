#ifndef VIRTUALMEM_H
#define VIRTUALMEM_H

#include "ata.h"

void handle_page_fault(void);
void setup_paging(void);
void mmap_disk(ata_disk *d);
#endif
