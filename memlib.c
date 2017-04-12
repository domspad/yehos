#include "asmhelpers.h"
#include "memlib.h"

void *
memcpy(void *_dest, const void *_src, int n)
{
    u8 *dest = (u8 *) _dest;
    u8 *src = (u8 *) _src;

    for (int i=0; i<n; i++)
       dest[i] = src[i];

    return dest;
}

void *
memset(void *_dest, int val, int length)
{
    int i = 0;
    u8 *dest = (u8 *) _dest;
    while(length > 0) {
        *dest = val;
        dest++;
        length--;
    }
    return dest;
}
