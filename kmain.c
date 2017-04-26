#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "ata.h"
#include "DiskFile.h"
#include "kernel.h"
#include "virtualmem.h"
#include "memlib.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);
void setup_timer(int timer_hz);

typedef int (*mainptr_t)(int argc, char **argv);

void
kmain(void)
{
    memset(START_BSS, 0, END_BSS - START_BSS);

    vga_cls();

    kprintf("sizeof(uint32_t)=%d, sizeof(uint64_t)=%d\n", sizeof(uint32_t), sizeof(uint64_t));

    setup_interrupts((void *) 0x1000);
    setup_paging();

    init_ata();
    ata_disk *d = &disks[0];
    mmap_disk(d);

    mmap("TXTPLAYR.BIN", 0x01000000);
    kprintf("first byte: %d\n", *(char *)0x01000000);

    mainptr_t entry = (mainptr_t) 0x01000000;
    (*entry)(0, NULL);
    kprintf("kernel initialized\n");
    setup_timer(30);

    while (1) yield();

}
