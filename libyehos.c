#include <ourtypes.h>

int READLINE_SCREEN_WIDTH = 80;
int readline_cursor_row = 0;
int readline_cursor_col = 0;

void *
mmap(void *addr, int length, int prot, int flags,
     const char *filename, int offset)
{
    int32_t ret;
    asm volatile ("pushl %1\n"
                  "pushl %2\n"
                  "mov $1, %%eax\n"
                  "int $0x30\n"
                  "add $8, %%esp\n"
                    : "=a" (ret)
                    : "r" (addr), "r" (filename)
                    : "memory");

    return (void *) ret;
}

#include "memlib.c"

char
read()
{
    int32_t ret;
    asm volatile("mov $2, %%eax\n"
                 "int $0x30\n"
                 : "=a" (ret)
                 :
                 : "memory");
    return (char) ret;
}

void
write(char c)
{
    asm volatile("pushl %0\n"
                 "mov $3, %%eax\n"
                 "int $0x30\n"
                 "add $4, %%esp\n"
                   :
                   : "r" ((int32_t) c)
                   : "memory");
    return;
}


void
setcursor(int x, int y)
{
    asm volatile(
            "pushl %0\n"
            "pushl %1\n"
            "mov $4, %%eax\n"
            "int $0x30\n"
            "add $8, %%esp\n"
            :
            : "r" ((int32_t) x), "r" ((int32_t) y)
            : "memory");
    return;
}


void
writechar(int x, int y, char c, char color)
{
    asm volatile(
            "pushl %0\n"
            "pushl %1\n"
            "pushl %2\n"
            "pushl %3\n"
            "mov $5, %%eax\n"
            "int $0x30\n"
            "add $16, %%esp\n"
            :
            : "r" ((int32_t) x), "r" ((int32_t) y), "r" ((int32_t) c), "r" ((int32_t) color)
            : "memory");
    return;
}

void clear_screen() {
    int col, row;
    for (row = 0; row < 25; row++) {
        for (col = 0; col < 80; col++){
            writechar(col, row, ' ', 0xf);
        }
    }
}

void _readline_cursor_next_line() {
    readline_cursor_row++;
    readline_cursor_col = 0;
    setcursor(readline_cursor_col, readline_cursor_row);
}

void _readline_cursor_next() {
    readline_cursor_col++;
    if (readline_cursor_col >= READLINE_SCREEN_WIDTH) {
        readline_cursor_row++;
        readline_cursor_col = 0;
    }
    setcursor(readline_cursor_col, readline_cursor_row);
}

void _readline_writechar(int x, int y, char c, char color) {
    writechar(x, y, c, color);
    _readline_cursor_next();
}

void _readline_delete() {
    if (readline_cursor_col == 0) {
        readline_cursor_row--;
        readline_cursor_col = 80;
    } else {
        readline_cursor_col--;
    }
    writechar(readline_cursor_col, readline_cursor_row, ' ', 0xf);
}

void
puts(char *s) {
    int len = strlen(s);

    for (int i = 0; i < len; i++) {
        if (s[i] == '\r' || s[i] == '\n') {
            _readline_cursor_next_line();
        } else {
            _readline_writechar(readline_cursor_col, readline_cursor_row, s[i], 0xf);
        }
    }
}

int
readline(char *buf) {
    int read_count = 0;
    int index = 0;

    _readline_writechar(readline_cursor_col, readline_cursor_row, '>', 0xf);
    _readline_writechar(readline_cursor_col, readline_cursor_row, ' ', 0xf);

    int BACKSPACE = 8;

    while(1){
        setcursor(readline_cursor_col, readline_cursor_row);
        char c = read();
        if (c == '\n' || c == '\r'){
            buf[index++] = c;
            _readline_cursor_next_line();
            buf[index] = '\0';
            return read_count;
        } else if (c == BACKSPACE) {
            if (index >= 0) {
                if (index > 0) _readline_delete();
                buf[index] = '\0';
                index--;
            }
        } else if (c > 0) {
            _readline_writechar(readline_cursor_col, readline_cursor_row, c, 0xf);
            buf[index++] = c;
            read_count++;
        } else {
            // TODO: put some printf debugging
        }
    }
    return read_count;
}
