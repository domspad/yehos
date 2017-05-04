#include "kernel.h"
#include "asmhelpers.h"
#include "virtualmem.h"
#include "globals.h"
#include "memlib.h"

// BEWARE
#define do_mmap mmap
#define SCREEN_WIDTH 80
#define SCREEN_LINE_SIZE (SCREEN_WIDTH * 2)

char KEYBOARD_BUFFER[MAX_KEYBOARD_BUFF];
int read_keyboard_index = 0;
int write_keyboard_index = 0;
int keyboard_buffer_full = 0;

int video_mem_index = 0;

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
            char *videomem = (char *) VIDEO_MEM;

            const int32_t *pParms = (const int32_t *) parms;
            videomem[video_mem_index] = (char) pParms[0];
            videomem[video_mem_index+1] = 0x0F;
            video_mem_index = video_mem_index + 2;

            return;
        }
    case 4: // setcursor
        {
            const int32_t *pParms = (const int32_t *) parms;
            int y = (int) pParms[0];
            int x = (int) pParms[1];
            u16 position = x + y * SCREEN_WIDTH;

            // cursor LOW port to vga INDEX register
            out8(0x3D4, 0x0F);
            out8(0x3D5, (unsigned char)(position&0xFF));
            // cursor HIGH port to vga INDEX register
            out8(0x3D4, 0x0E);
            out8(0x3D5, (unsigned char )((position>>8)&0xFF));

            return;
        }
    case 5: // writechar
        {
            char *videomem = (char *) VIDEO_MEM;

            const int32_t *pParms = (const int32_t *) parms;
            char color = (char) pParms[0];
            char c = (char) pParms[1];
            int y = (int) pParms[2];
            int x = (int) pParms[3];
            int index = x * 2 + y * SCREEN_LINE_SIZE;
            videomem[index] = c;
            videomem[index+1] = color;

            return;
        }
    case 6: // scroll
        {
            char *videomem = (char *) VIDEO_MEM;
            memmove(videomem, videomem + SCREEN_LINE_SIZE, 24 * SCREEN_LINE_SIZE);
            for (int i = 0; i < SCREEN_WIDTH; i++) {
                videomem[i * 2 + 24 * SCREEN_LINE_SIZE] = ' ';
                videomem[i * 2 + 24 * SCREEN_LINE_SIZE + 1] = 0xf;
            }

            return;
        }
    default:
        kprintf("system call %d not supported\n", syscall_num);
        break;
    };
}
