#include <ourtypes.h>

void *
mmap(void *addr, int length, int prot, int flags,
     const char *filename, int offset)
{
    int32_t ret;
    asm volatile ("pushl %1\n"
                  "pushl %2\n"
                  "mov $1, %%eax\n"
                  "int $0x30\n"
                    : "=a" (ret)
                    : "r" (addr), "r" (filename)
                    : "memory");

    return (void *) ret;
}

#include "memlib.c"
