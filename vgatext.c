#include "asmhelpers.h"

static volatile char *videomem = (volatile char *) 0xb8000;
void
vga_putc(u8 ch, u8 color)
{
    static int index = 0;

    videomem[index*2] = ch;
    videomem[index*2+0x01] = color;
    index++;
}

void
vga_cls(void)
{
    int j;
    while(j < 0x06400) {
        videomem[j] = ' ';
        videomem[j+1] = 0x00;
        j++;
        j++;
    }
}
