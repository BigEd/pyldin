#ifndef _FONT_H_
#define _FONT_H_

typedef struct {
    char *name;
    short width;
    short height;
    short ascent;
    short first;
    short last;
    unsigned char *bits;
    unsigned short *offset;
    unsigned short fixed_width;
    unsigned short fixed_height;
    unsigned int bits_size;
} DOS_FONT;

#endif
