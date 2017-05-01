#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DiskFile.h"
#include "iso9660.h"
#include "kernel.h"

DiskFile g_files[256];

void *sector(void *baseptr, unsigned int sectornum)
{
    u8 *cdimg = baseptr;
    return &cdimg[sectornum * 2048];
}

const DirectoryRecord *get_first_entry(void *baseptr)
{
    
    const PrimaryVolumeDescriptor *pvd = sector(baseptr, 16);

    const DirectoryRecord *rootrecord = &pvd->root_directory_record;
    assert(rootrecord->record_len == sizeof(DirectoryRecord) + rootrecord->id_len);

    const DirectoryRecord *entry = sector(baseptr, rootrecord->data_sector);
    return entry;
}

DiskFile * iso9660_enumfiles(void *baseptr)
{
    const DirectoryRecord *entry = get_first_entry(baseptr);
    int i=0;
    do {
        DiskFile *fp = &g_files[i++];
        strncpy(fp->filename, entry->id, entry->id_len);
        fp->data = sector(baseptr, entry->data_sector);
        fp->length = entry->data_len;

        printf("CD file: %s at 0x%x (%u bytes)\n",
                    fp->filename, fp->data, fp->length);

        entry = NEXT_DIR_ENTRY(entry);
    } while (entry->record_len > 0);

    g_files[i].filename[0] = 0;
    g_files[i].data = NULL;
    g_files[i].length = 0;

    return g_files;
}

DiskFile *
iso9660_fopen_r(void *baseptr, const char *filename)
{
    DiskFile *files = iso9660_enumfiles(baseptr);

    int i=0;
    for ( ; files[i].data; ++i)
    {
        DiskFile *df = &files[i];
        if (strncmp(df->filename, filename, sizeof(filename)) == 0)
        {
            printf("found file '%s'\n", df->filename);
            return df;
        }
        else
        {
            printf("skipping file '%s'\n", df->filename);
        }
    }
    return NULL;
}

DiskFile *
iso9660_fopen_w(void *baseptr, const char *filename)
{
    return NULL;
}

