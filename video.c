#include "memlib.h"
#include "ourtypes.h"
#include "vgatext.h"
#include "kernel.h"
#include "asmhelpers.h"
#include "globals.h"

static volatile char *videomem = (volatile char *) 0xb8000;

void show_image(char * pic_index, int size) {
    memcpy((void *) videomem, pic_index, size);
}

char *pic_index = 0x0;

void play_video(void) {

    int last_timer_index = timer_index;
    int img_number = 0;

    while(1) {
        if (last_timer_index != timer_index) {
            last_timer_index = timer_index;
            if(!pause_set) {
                show_image(pic_index+img_number*4000, 4000);
                img_number++;
                vga_setchar(79, 0, (u8) img_number+'0', 0x03);
            }
        }
    }


}
