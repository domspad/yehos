extern char START_BSS[], END_BSS[];

void
kmain(void)
{
//    memset(START_BSS, 0, END_BSS - START_BSS);

    char *videomem = (char *) 0xb8000;
    videomem[0] = 'F';
    videomem[1] = 0x14;
}
