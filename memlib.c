#include "asmhelpers.h"

void *
memcpy(void *_dest, const void *_src, int n)
{
    u8 *dest = (u8 *) _dest;
    u8 *src = (u8 *) _src;

    for (int i=0; i<n; i++)
       dest[i] = src[i];

    return dest;
}
