#ifndef KERNEL_H
#define KERNEL_H

void kprintf(const char *fmt, ...);
#define DPRINT(L, FMTSTR, args...) kprintf(FMTSTR "\n", ##args)

#endif
