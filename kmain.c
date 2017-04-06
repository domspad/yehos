#include "asmhelpers.h"

extern char START_BSS[], END_BSS[];

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);
    int index;
    char a[] = {'?', '?', '1', '2', '3', '4', '5', '6','7','8','9', '0', '-', '=','?','?','q','w','e','r','t','y','u','i','o','p','[',']','?','?','a','s','d','f','g','h','j','k','l',';','\'','`','?','\\','z','x','c','v','b','n','m',',','.','/','?','?','?',' ','?','?','?','?','?','?','?','?','?','?','?','?','?'};

    char *videomem = (char *) 0xb8000;
    char *keyboardmem = (char *) 0xfe00;
    char *statusport = (char *) 0x60;
    char *keyboard = (char *) 0x64;
    char val;
    u8 c, d;
    //videomem[0] = 'F';
    //videomem[1] = in8(0xd4);
    // This is
    int j = 0;
    char * video = videomem;
    while(j < 0x06400){
        video[j] = ' ';
        video[j+1] = 0x00;
        j++;
        j++;
    }
    while(1){
        //if(keyboardmem[0] == 1){
        while(!((c = in8(0x64)) & 0x01)){}
        d = in8(0x60);
        if (d > 127) {
            d -= 128;
            val = ' ';
        }
        else {
             val = a[d];
        }
        /*if(d % 2 == 0)*/
            /*index = d;*/
        /*else*/
            /*index = d + 1;*/
        index = d;
        videomem[index*2] = val;
        videomem[index*2+0x01] = 0x07;

    }
}
