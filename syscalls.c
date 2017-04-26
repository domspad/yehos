#include "kernel.h"
#include "asmhelpers.h"
#include "virtualmem.h"

// BEWARE
#define do_mmap mmap

int
syscall_handler(int syscall_num, const void *parms)
{
    switch (syscall_num) {
    case 0: // _exit
        {
            const int32_t *pRetcode = (const int32_t *) parms;
            int retcode = *pRetcode;
            kprintf("application exited with return code %d\n", retcode);
            while (1) yield();
        }
        break;

    case 1: // mmap
        {
            const int32_t *pParms = (const int32_t *) parms;
            int r = do_mmap(pParms[1], pParms[0]);
            if (r < 0) {
                return -1;
            } else {
                return pParms[1];
            }
        }
    default:
        kprintf("system call %d not supported\n", syscall_num);
        break;
    };
}
