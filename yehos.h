#ifndef YEHOS_H_
#define YEHOS_H_

static const int READLINE_BUFFER_SIZE = 512;
// header file for applications to include to get yehos' system calls

void *
mmap(void *addr, int length, int prot, int flags,
     const char *filename, int offset);

char
read();

void
write(char c);

void
writechar(int x, int y, char c, char color);

void
scroll();

void
setcursor(int x, int y);

int
readline(char *buffer); // buffer is assumed to have READLINE_BUFFER_SIZE

void
clear_screen();

void
puts(char *s);
#endif
