#ifndef YEHOS_H_
#define YEHOS_H_

// header file for applications to include to get yehos' system calls

void *
mmap(void *addr, int length, int prot, int flags,
     const char *filename, int offset);

#endif
