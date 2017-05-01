#include "asmhelpers.h"
#include "memlib.h"

void *
memcpy(void *_dest, const void *_src, size_t n)
{
    u8 *dest = (u8 *) _dest;
    u8 *src = (u8 *) _src;

    for (int i=0; i<n; i++)
       dest[i] = src[i];

    return dest;
}

void *
memset(void *_dest, int val, size_t length)
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

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *pd = dest;
    const unsigned char *ps = src;
    if (ps < pd)
        for (pd += n, ps += n; n--;)
            *--pd = *--ps;
    else
        while(n--)
            *pd++ = *ps++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *us1 = (const unsigned char *) s1;
    const unsigned char *us2 = (const unsigned char *) s2;
    while (n-- != 0) {
        if (*us1 != *us2)
            return (*us1 < *us2) ? -1 : +1;
        us1++;
        us2++;
    }
    return 0;
}
int strncmp(const char *s1, const char *s2, size_t n)
{
     unsigned char uc1, uc2;
     /* Nothing to compare?  Return zero.  */
     if (n == 0)
         return 0;
     /* Loop, comparing bytes.  */
     while (n-- > 0 && *s1 == *s2) {
         /* If we've run out of bytes or hit a null, return zero
            since we already know *s1 == *s2.  */
         if (n == 0 || *s1 == '\0')
             return 0;
         s1++;
         s2++;
     }
     uc1 = (*(unsigned char *) s1);
     uc2 = (*(unsigned char *) s2);
     return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    do {
        if (!n--)
            return ret;
    } while ((*dest++ = *src++));
    while (n--)
        *dest++ = 0;
    return ret;
}
int strlen(const char *s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strcpy(char * dest, const char * src)
{
    char *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

int
strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2; s1++, s2++)
        if (*s1 == '\0')
            return 0;
    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
}
