#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

uint8_t parse_esc_color(FILE *fp);

int width = 80;
int height = 25;

int
main(int argc, const char *argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    FILE *fpout = fopen(argv[2], "wb");
    int ch;
    uint8_t color;
    int col = 0;
    int lines = 0;

    while ((ch = fgetc(fp)) != EOF) {
        if (lines >= height) {
            break;
        }
        switch (ch) {
        case 27:
            color = parse_esc_color(fp);
            break;
        case '\r':
            break;
        case '\n':
            if (col == 0 && lines == 0) {
                // skip empty leading lines
                continue;
            }
            for (int i=col; i < width; i++) {
                fputc(0, fpout);
                fputc(color, fpout);
            }
            lines++;
            col = 0;
            break;
        default:
            fputc(ch, fpout);
            fputc(color, fpout);
            col++;
            break;
        }
    }
    fclose(fp);
    fclose(fpout);

    printf("%d lines written\n", lines);
    return 0;
}

uint8_t
parse_esc_color(FILE *fp)
{
    uint8_t color = 0;

    char escseq[16];
    int i = 0;

    int ch = fgetc(fp);
    assert(ch == '[');

    while (1) {
        ch = fgetc(fp);
        if (ch == 'm') {
            break;
        } else if (ch == ';') {
            escseq[i] = 0;
            i = 0;
            int parm = atoi(escseq);
            if (parm >= 40) {
                color &= 0x0f;
                color |= (parm - 40) << 4;
            } else if (parm >= 30) {
                color &= 0xf0;
                color |= (parm - 30);
            } else if (parm == 1) {
                color |= 0x08;
            } else if (parm == 5) {
                color = 0x80;
            } else if (parm == 0) {
                color = 0x00;
            } else {
                printf("unsupported color code %d\n", parm);
                assert(0);
            }
        } else {
            escseq[i++] = ch;
        }
    }
    return color;
}
