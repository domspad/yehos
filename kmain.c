#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "memlib.h"
#include "video.h"
#include "ata.h"
#include "DiskFile.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);

static volatile char *videomem = (volatile char *) 0xb8000;

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);

//    setup_interrupts((void *) 0x1000);
    init_ata();
    ata_disk *d = &disks[0];

    // read entire .iso into memory at the 1MB mark
    atapi_read_lba(d, (void *) 0x100000, 0xFFFF, 0, d->max_lba);

    vga_cls();

    DiskFile * df = iso9660_fopen_r((void *) 0x100000, "SW78.VGA");
    while (1) yield();
    show_image(df->data, 4000);

    vga_putc('>', 0x07);
}
