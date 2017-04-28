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
                  "add $8, %%esp\n"
                    : "=a" (ret)
                    : "r" (addr), "r" (filename)
                    : "memory");

    return (void *) ret;
}

#include "memlib.c"

char
read()
{
    int32_t ret;
    asm volatile("mov $2, %%eax\n"
                 "int $0x30\n"
                 : "=a" (ret)
                 :
                 : "memory");
    return (char) ret;
}

void
write(char c)
{
    asm volatile("pushl %0\n"
                 "mov $3, %%eax\n"
                 "int $0x30\n"
                 "add $4, %%esp\n"
                   :
                   : "r" ((int32_t) c)
                   : "memory");
    return;
}


void
setcursor(int x, int y)
{
    asm volatile(
            "pushl %0\n"
            "pushl %1\n"
            "mov $4, %%eax\n"
            "int $0x30\n"
            "add $8, %%esp\n"
            :
            : "r" ((int32_t) x), "r" ((int32_t) y)
            : "memory");
    return;
}


void
writechar(int x, int y, char c, char color)
{
    asm volatile(
            "pushl %0\n"
            "pushl %1\n"
            "pushl %2\n"
            "pushl %3\n"
            "mov $5, %%eax\n"
            "int $0x30\n"
            "add $16, %%esp\n"
            :
            : "r" ((int32_t) x), "r" ((int32_t) y), "r" ((int32_t) c), "r" ((int32_t) color)
            : "memory");
    return;
}
