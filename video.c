#include "memlib.h"
static volatile char *videomem = (volatile char *) 0xb8000;

void show_image(char * pic_index, int size) {
    memcpy((void *) videomem, pic_index, size);
}
