#ifndef VGATEXT_H
#define VGATEXT_H

void vga_setchar(int x, int y, u8 ch, u8 color);
void vga_putc(u8 ch, u8 color);
void vga_cls(void);

#endif
