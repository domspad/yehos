#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "video.h"
#include "ata.h"
#include "DiskFile.h"
#include "kernel.h"
#include "virtualmem.h"
#include "memlib.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);
void setup_timer(int timer_hz);

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
    kprintf("got to here\n");
    mmap("STARWARS.VGA", 0x70000000);
    setup_timer(30);
    play_video(0x70000000 + 4000 * 200);

    while (1) yield();

}
