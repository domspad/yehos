#include "asmhelpers.h"
#include "kb.h"

extern char START_BSS[], END_BSS[];
u16 scancode_to_ascii(u8 scancode);

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);
    int index=0;

    char *videomem = (char *) 0xb8000;
    char *keyboardmem = (char *) 0xfe00;
    char *statusport = (char *) 0x60;
    char *keyboard = (char *) 0x65;
    u16 val;
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
        while(!((c = in8(0x64)) & 0x01)){}
        u8 scancode = in8(0x60);
        val = scancode_to_ascii(scancode);

        if(val == 0) continue;

        videomem[index*2] = val;
        videomem[index*2+0x01] = 0x07;
        index++;
	}

}
