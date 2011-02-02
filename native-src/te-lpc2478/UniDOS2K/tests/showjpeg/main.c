#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gba-jpeg-decode.h"

int main(int argc, char* argv[]) {
    int size;
    unsigned char *image;
    FILE *f;

    if (argc < 2) {
        printf("Usage: %s <input.jpg>\n", argv[0]);
        return 2;
    }
    f = fopen(argv[1], "rb");
    if (!f) {
        printf("Error opening the input file.\n");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    size = (int) ftell(f);
    image = malloc(size);
    fseek(f, 0, SEEK_SET);
    size = (int) fread(image, 1, size, f);
    printf("Filesize: %d\n", size);
    fclose(f);

    JPEG_DecompressImage(image, (unsigned short *)0xa0000000, 320, 240);

    free(image);

    return 0;
}
