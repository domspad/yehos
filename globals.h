#ifndef GLOBALS_H
#define GLOBALS_H

// anything that can be changed between instructions
// should be volatile (e.g. via interrupt)
extern volatile int timer_index;
extern volatile int pause_set;
extern volatile int seek;

extern char *pic_index;

#endif
