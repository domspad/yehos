#include <ourtypes.h>

int READLINE_SCREEN_WIDTH = 80;
int readline_cursor_row = 0;
int readline_cursor_col = 0;

/* inline requests the compiler to replace function call with actual function code
 * (to avoid the overhead of function setup and teardown) */
inline int32_t syscall(int syscall_num, const int32_t *parms)
{
    int32_t ret;
    /* put syscall_num in eax (same as where return val will come) and parms pointer in ebx*/
    asm volatile(
            "int $0x30\n"
            : "=a" (ret)
            : "0" (syscall_num), "b" (parms)
            : "memory");
    return ret;
}

void *
mmap(void *addr, int length, int prot, int flags,
     const char *filename, int offset)
{
    int32_t parms[] = { (int32_t) filename, (int32_t) addr };
    return (void *) syscall(1, parms);
}

#include "memlib.c"

char
read()
{
    return (char) syscall(2, NULL);
}

void
write(char c)
{
    int32_t parms[] = { c };
    (void) syscall(3, parms);
}

void
setcursor(int x, int y)
{
    int32_t parms[] = { y, x };
    (void) syscall(4, parms);
}

void
writechar(int x, int y, char c, char color)
{
    int32_t parms[] = { color, c, y, x };
    (void) syscall(5, parms);
}

void
scroll()
{
    (void) syscall(6, NULL);
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
    if (readline_cursor_row < 24) {
        readline_cursor_row++;
    } else {
        scroll();
    }
    readline_cursor_col = 0;
    setcursor(readline_cursor_col, readline_cursor_row);
}

void _readline_cursor_next() {
    readline_cursor_col++;
    if (readline_cursor_col >= READLINE_SCREEN_WIDTH) {
        _readline_cursor_next_line();
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
            _readline_cursor_next_line();
            buf[index] = '\0';
            return read_count;
        } else if (c == BACKSPACE) {
            if (index >= 0) {
                if (index > 0) {
                    _readline_delete();
                    index--;
                }
                buf[index] = '\0';
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
