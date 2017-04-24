

int
main()
{
    char *videomem = (char *) 0xb8000;
    videomem[0] = 'H';
    videomem[1] = 0x07;

    return 0;
}
