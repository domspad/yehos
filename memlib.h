#ifndef MEMLIB_H
#define MEMLIB_H

#include <string.h>


void * memcpy(void *_dest, const void *_src, size_t n);

void * memset(void *_dest, int val, size_t length);

void *memmove(void *dest, const void *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);

char *strncpy(char *dest, const char *src, size_t n);
int strlen(const char *s);

char *strchr(const char *s, int c);

char *strcat(char *dest, const char *src);

char *strcpy(char * dest, const char * src);
#endif
