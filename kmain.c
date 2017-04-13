#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "memlib.h"
#include "video.h"
#include "ata.h"
#include "DiskFile.h"
#include "kernel.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);
void setup_timer(int timer_hz);

static volatile char *videomem = (volatile char *) 0xb8000;
extern char *pic_index;

void
kmain(void)
{
    memset(START_BSS, 0, END_BSS - START_BSS);

    vga_cls();
    kprintf("sizeof(uint32_t)=%d, sizeof(uint64_t)=%d\n", sizeof(uint32_t), sizeof(uint64_t));
    init_ata();
    ata_disk *d = &disks[0];

    kprintf("#LBA on disk 0 = %d\n", d->max_lba);

    // read entire .iso into memory at the 1MB mark
    for (uint32_t i=0; i < d->max_lba; i++) {
        char *diskbuf = (void *) 0x90000;
        while (atapi_read_lba(d, diskbuf, 0xFFFF, i, 1) < 0) {
            kprintf("timeout on sector %d\n", i);
        }
        memcpy((void *) (0x100000 + i*2048), diskbuf, 2048);
        vga_putc('.', 0x07);
    }

    DiskFile * df = iso9660_fopen_r((void *) 0x100000, "STARWARS.VGA");
    pic_index = df->data + 4000*200;
    setup_timer(30);
    setup_interrupts((void *) 0x1000);

    play_video();
}
