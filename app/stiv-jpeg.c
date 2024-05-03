/* tiv - terminal image viewer - copyleft 2013-2015 - pancake */

#include <string.h>
#include <stdlib.h>

#include <stiv.h>

int main(int argc, const char** argv) {
    stivctx_t* ctx;
    uint32_t width = 80;
    stiv_mode mode = STIV_MODE_RGB;

    if (argc < 2) {
        printf("stiv-jpeg . suckless terminal image viewer\n"
            "Usage: stiv [image] [width] [mode]\n"
            "Modes: [ascii,ansi,grey,256,rgb]\n");
        return 1;
    }

    if (argc > 2) {
        width = atoi(argv[2]);
    }

    if (argc > 3) {
        if (!strcmp(argv[3], "ascii")) mode = STIV_MODE_ASCII;
        else if (!strcmp(argv[3], "ansi")) mode = STIV_MODE_ANSI;
        else if (!strcmp(argv[3], "grey")) mode = STIV_MODE_GREY;
        else if (!strcmp(argv[3], "256")) mode = STIV_MODE_256;
        else if (!strcmp(argv[3], "rgb")) mode = STIV_MODE_RGB;
    }

    if (!(ctx = stiv_from_jpeg(argv[1], width, 0, mode))) {
        printf("Failed to load image\n");
        return 1;
    }

    stiv_display(ctx);

    stiv_free(ctx);

    return 0;
}