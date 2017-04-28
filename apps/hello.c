#include <yehos.h>

int
main()
{
    clear_screen();
    int vid_index = 0;
    int cursor_position = 0;
    while(1){
        setcursor(cursor_position, 0);
        int a = read();
        if(a > 0){
            write(a);
            cursor_position += 1;
        }
    }
    return 25;
}
