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
volatile int seek = 0;

void play_video(void) {

    int last_timer_index = timer_index;

    while(1) {
        yield();
        if (last_timer_index != timer_index) {
            last_timer_index = timer_index;
            if(!pause_set) {
                show_image(pic_index+seek*4000, 4000);
                seek++;
            }

            // boundaries
            if(seek < 0) seek = 0;
            if (seek > 13460) seek=13460;
        }
    }


}
