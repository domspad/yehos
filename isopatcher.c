#include <stdio.h>
#include <assert.h>
#include "DiskFile.h"
#include "iso9660.h"

char buf[1024*1024];

int
main(int argc, const char *argv[])
{
    FILE *fp = fopen(argv[1], "r+b");
    if (fp == NULL) {
        perror(argv[1]);
        return -1;
    }
    size_t nbytes = fread(buf, 1, 1024*1024, fp);
    struct {
        uint16_t lba_kernel;
        uint16_t num_sects;
    } info;

    DiskFile *kernel = iso9660_fopen_r(buf, "KERNEL.BIN");
    assert(kernel);

    info.lba_kernel = ((int) kernel->data - (int) buf) / 512;
    info.num_sects = kernel->length / 512 + 1;

    assert(sizeof(info) == 4);
    fseek(fp, 0, SEEK_SET);
    size_t r = fwrite(&info, sizeof(info), 1, fp);
    fclose(fp);

    return 0;
}
