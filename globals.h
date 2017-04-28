#ifndef GLOBALS_H
#define GLOBALS_H

// anything that can be changed between instructions
// should be volatile (e.g. via interrupt)
extern volatile int timer_index;
extern volatile int pause_set;
extern volatile int seek;

extern char *pic_index;
#define MAX_KEYBOARD_BUFF 256
extern char KEYBOARD_BUFFER[MAX_KEYBOARD_BUFF];
extern int read_keyboard_index;
extern int write_keyboard_index;
extern int keyboard_buffer_full;

#define VIDEO_MEM 0xb8000
extern int video_mem_index;

#endif
