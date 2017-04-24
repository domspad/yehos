#include "kernel.h"
#include "asmhelpers.h"

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
#if 0
    case 1: // mmap
        {
            const mmap_parms_t *pParms = (const mmap_parms_t *) parms;
            int r = do_mmap(pParms->filename, pParms->virtaddr);
            return r;
        }
#endif
    default:
        kprintf("system call %d not supported\n", syscall_num);
        break;
    };
}
