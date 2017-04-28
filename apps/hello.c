#include <yehos.h>

void clear_screen() {
    int col, row;
    for (row = 0; row < 25; row++) {
        for (col = 0; col < 80; col++){
            writechar(col, row, ' ', 0xf);
        }
    }
}

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
