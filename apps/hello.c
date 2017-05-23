#include <yehos.h>

int
main()
{
    clear_screen();
    char *buf[READLINE_BUFFER_SIZE];
    while (1) {
        int read_count = readline(buf);
        if (read_count > 0) {
            puts(buf);
            puts("\n");
        }
    }
    return 25;
}
