#include <yehos.h>
#include <string.h>

#include "video.h"

#define MOVIE_ADDR 0x70000000

int
main(int argc, char **argv)
{
    void *ptr = mmap((void *)MOVIE_ADDR, -1, 0, 0, "STARWARS.VGA", 0);
    play_video(ptr + 4000 * 200);

    _exit(0);
}
