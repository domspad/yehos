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

static volatile char *videomem = (volatile char *) 0xb8000;
extern char *pic_index;

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);

    vga_cls();
    kprintf("sizeof(uint32_t)=%d, sizeof(uint64_t)=%d\n", sizeof(uint32_t), sizeof(uint64_t));
    init_ata();
    ata_disk *d = &disks[0];

    // read entire .iso into memory at the 1MB mark
    for (uint32_t i=0; i < 256; i++)
        atapi_read_lba(d, (void *) (0x100000 + i*2048), 0xFFFF, i, 1); //d->max_lba);

    DiskFile * df = iso9660_fopen_r((void *) 0x100000, "SW78.VGA");
    pic_index = df->data;

    setup_interrupts((void *) 0x1000);
    vga_putc('>', 0x07);
}
