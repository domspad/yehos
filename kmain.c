#include "asmhelpers.h"
#include "kb.h"
#include "vgatext.h"

extern char START_BSS[], END_BSS[];
u16 scancode_to_ascii(u8 scancode);
void setup_interrupts(void *idtaddr);

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
    //videomem[0] = 'F';
    //videomem[1] = in8(0xd4);
    // This is
    int j = 0;

    setup_interrupts((void *) 0x1000);
    vga_cls();

    vga_putc('>', 0x04);

    while(1){
        while(!((c = in8(0x64)) & 0x01)){}
        u8 scancode = in8(0x60);
        val = scancode_to_ascii(scancode);

        if(val == 0) continue;

        vga_putc(val, 0x07);

        if(val == '`') {
            vga_putc(val/0, 0x07);
        }
	}

}
