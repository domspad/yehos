#include "kernel.h"
#include "asmhelpers.h"
#include "virtualmem.h"
#include "globals.h"

// BEWARE
#define do_mmap mmap

char KEYBOARD_BUFFER[MAX_KEYBOARD_BUFF];
int read_keyboard_index = 0;
int write_keyboard_index = 0;
int keyboard_buffer_full = 0;

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
            int r = do_mmap(pParms[0], pParms[1]);
            if (r < 0) {
                return -1;
            } else {
                return pParms[1];
            }
        }
    case 2: // read
        {
            // check if buffer empty
            if (!keyboard_buffer_full && read_keyboard_index == write_keyboard_index){
                kprintf("nothing in buffer yo\n");
                return -1;
            } else {
                keyboard_buffer_full = 0;
                char ret = KEYBOARD_BUFFER[read_keyboard_index++];

                read_keyboard_index = read_keyboard_index % MAX_KEYBOARD_BUFF;
                return ret;
            }
        }
    case 3: // write
        {

        }
    default:
        kprintf("system call %d not supported\n", syscall_num);
        break;
    };
}
