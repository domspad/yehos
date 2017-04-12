#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"
#include "memlib.h"
#include "video.h"

extern char START_BSS[], END_BSS[];
void setup_interrupts(void *idtaddr);

static volatile char *videomem = (volatile char *) 0xb8000;

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);
    int index=0;


    char *keyboardmem = (char *) 0xfe00;
    char *statusport = (char *) 0x60;
    char *keyboard = (char *) 0x65;
    u16 val;
    u8 c, d;

    setup_interrupts((void *) 0x1000);
    vga_cls();

    vga_putc('>', 0x04);

}
