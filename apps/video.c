#include <yehos.h>

static volatile char *videomem = (volatile char *) 0xb8000;

void show_image(char * pic_index, int size) {
    memcpy((void *) videomem, pic_index, size);
}

volatile int seek = 0;

void play_video(char * pic_index) {

    show_image(pic_index, 4000);
    int last_timer_index = get_timer_index();

    while(1) {
        yield();
        if (last_timer_index != get_timer_index()) {
            last_timer_index = get_timer_index();
            show_image(pic_index + seek * 4000, 4000);
            seek++;

            // boundaries
            if (seek < 0) seek = 0;
            if (seek > 13460) seek = 13460;
        }
    }

}
