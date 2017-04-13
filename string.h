#ifndef STRING_H
#define STRING_H

#include <stdlib.h> // size_t

int strlen(const char *);
char *strchr(const char *s, int c);
char *strcat(char *dest, const char *src);
char *strdup(const char *src);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int snprintf(char *buf, unsigned int sz, const char *fmt, ...);



#endif
