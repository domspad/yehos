#include "asmhelpers.h"
#include "vgatext.h"
#include "memlib.h"

static volatile char *videomem = (volatile char *) 0xb8000;
void
vga_putc(u8 ch, u8 color)
{
    static int index = 0;

    if (ch == '\n') {
        index = (index / 160) * 160 + 160;
        return;
    }
    videomem[index*2] = ch;
    videomem[index*2+0x01] = color;
    index++;
}

void
vga_setchar(int x, int y, u8 ch, u8 color)
{
    videomem[y*160+x*2] = ch;
    videomem[y*160+x*2+1] = color;
}

void
vga_cls(void)
{
    memset((void *)videomem, 0x00, 0x06400);
}

void vga_putstr(const char *str)
{
    while (*str) {
        vga_putc(*str++, 0x07);
    }
}
