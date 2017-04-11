#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

uint8_t parse_esc_color(const char *ansicmd);
void test_colors(void);

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
    char buf[16];
    int i;

    test_colors();

    while ((ch = fgetc(fp)) != EOF) {
        if (lines >= height) {
            break;
        }
        switch (ch) {
        case 27:
            for (i=0; !isalpha((ch = fgetc(fp))); i++) {
                buf[i] = ch;
            }
            buf[i] = 0;
            color = parse_esc_color(buf);
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

uint8_t vga_colors[] = { 0x0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7 };

uint8_t
parse_esc_color(const char *colorstr)
{
    static uint8_t color = 0;

    char escseq[16];
    int i = 0;

    int ch = *colorstr++;
    assert(ch == '[');

    while (1) {
        ch = *colorstr++;
        if (!ch || ch == ';') {
            escseq[i] = 0;
            i = 0;
            int parm = atoi(escseq);
            if (parm >= 40) {
                color &= 0x8f;
                int vga_color = vga_colors[(parm - 40)];
                color |= vga_color << 4;
            } else if (parm >= 30) {
                color &= 0xf8;
                int vga_color = vga_colors[(parm - 30)];
                color |= vga_color;
            } else if (parm == 1) {
                color |= 0x08;
            } else if (parm == 5) {
                color |= 0x80;
            } else if (parm == 0) {
                color = 0x07;
            } else {
                printf("unsupported color code %d\n", parm);
                assert(0);
            }
            if (!ch) {
                return color;
            }
        } else {
            escseq[i++] = ch;
        }
    }
    return color;
}

void
test_colors()
{
    assert(parse_esc_color("[0;1;30;40") == 0x08);
}
