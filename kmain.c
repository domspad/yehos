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

#define PAGEDIR_ADDR 0x80000
#define PT0_ADDR 0x81000

void
kmain(void)
{
    memset(START_BSS, 0, END_BSS - START_BSS);

    vga_cls();

    kprintf("sizeof(uint32_t)=%d, sizeof(uint64_t)=%d\n", sizeof(uint32_t), sizeof(uint64_t));

    setup_interrupts((void *) 0x1000);

    // set up paging

    uint32_t *pagedir = (void *) PAGEDIR_ADDR;
    memset(pagedir, 0, 4096);
    pagedir[0] = PT0_ADDR | 0x03;
    pagedir[1023] = PAGEDIR_ADDR | 0x03;

    uint32_t *pt0 = (void *) PT0_ADDR;
    memset(pt0, 0, 4096);

    // identity-map first 1MB
    for (unsigned int i=0; i<256; ++i) {
        pt0[i] = (i << 12) | 0x03;
    }

//    pt0[7] = 0xFFFFFFF0;

    set_cr3(PAGEDIR_ADDR);

    set_cr0_paging();

    kprintf("got here\n");

    uint32_t *foo = (uint32_t *) 0x400000;

    *foo = 0xdeadbeef;
    while (1) yield();

#if 0
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

    play_video();
#endif
}
