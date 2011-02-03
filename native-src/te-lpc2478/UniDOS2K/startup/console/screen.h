#ifndef _SCREEN_H_
#define _SCREEN_H_

void screen_init(void *videomem, int width, int height);

void screen_clear(void);

void screen_fgcolor(unsigned short col);

void screen_bgcolor(unsigned short col);

void screen_putchar(unsigned char c, int x, int y);

void screen_get_size(int *w, int *h);

void screen_get_text_size(int *w, int *h);

void screen_scroll_up(void);

#endif
