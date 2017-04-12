#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "memlib.h"
#include "video.h"
#include "ata.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);

static volatile char *videomem = (volatile char *) 0xb8000;

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);

    setup_interrupts((void *) 0x1000);
//    init_ata();
//    ata_disk *d = &disks[0];

//    atapi_read_lba(d, (void *) 0x100000, 4096, 0);

    vga_cls();

//    show_image((void *) 0x100000, 512);

    vga_putc('>', 0x04);

}
