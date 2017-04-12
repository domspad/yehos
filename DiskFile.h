#ifndef DISKFILE_H_
#define DISKFILE_H_

#include <stdint.h>

typedef struct DiskFile {
    char filename[64];
    void *data;
    uint64_t length;
} DiskFile;

DiskFile * iso9660_enumfiles(void *baseptr);
DiskFile * iso9660_fopen_r(void *baseptr, const char *filename);
DiskFile * iso9660_fopen_w(void *baseptr, const char *filename);

#endif
