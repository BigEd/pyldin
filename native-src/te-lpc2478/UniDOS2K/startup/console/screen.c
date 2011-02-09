#include <inttypes.h>
#include <string.h>
#include "font.h"
#include "koi5x8.h"
#include "screen.h"
#include "../config.h"
#include "../lcd.h"
#include "cursor.h"

#warning FIX IT!
static unsigned short *screen = (unsigned short *)0xa0000000;
static int screen_width = 320;
static int screen_height = 240;

static int max_text_width = 0;
static int max_text_height = 0;

static const DOS_FONT *font;

static unsigned short fg_color;
static unsigned short bg_color;

void screen_init(void)
{
    LCD_Init(LCD_BUFFER_ADDR, NULL);

    LCD_Cursor_Dis(0);
    LCD_Copy_Cursor ((unsigned int *)Cursor, 0, sizeof(Cursor)/sizeof(uint32_t));
    LCD_Cursor_Cfg(CRSR_FRAME_SYNC | CRSR_PIX_32);
    LCD_Cursor_En(0);
//    LCD_Move_Cursor(0, 0);
    LCD_Ctrl(1);

    screen = (unsigned short *) LCD_BUFFER_ADDR;
    screen_width = 320;
    screen_height = 240;

    font = &font_koi5x8;

    max_text_width = screen_width / font->width;
    max_text_height = screen_height / font->height;

    fg_color = 0x7fff;
    bg_color = 0;

    screen_clear();
}

void screen_clear(void)
{
    unsigned short *addr = screen;
    int i;
    for (i = 0; i < screen_width * screen_height; i++)
	*addr++ = bg_color & 0x7fff;
}

void screen_fgcolor(unsigned short col)
{
    fg_color = col;
}

void screen_bgcolor(unsigned short col)
{
    bg_color = col;
}

void screen_putchar(unsigned char c, int x, int y)
{
    if (x < max_text_width && y < max_text_height) {
	unsigned short *addr = screen + y * font->height * screen_width + x * font->width;
	int offset = font->offset[c];
	int i, j;
	for (j = 0; j < font->height; j++) {
	    unsigned char b = font->bits[offset + j];
	    for (i = 0; i < font->width; i++) {
		if (b & 0x80)
		    addr[i] = fg_color;
		else if (!(bg_color & 0x8000))
		    addr[i] = bg_color;
		b <<= 1;
	    }
	    addr += screen_width;
	}
    }
}

void screen_cursor_enable(int en)
{
    if (en)
	LCD_Cursor_En(0);
    else
	LCD_Cursor_Dis(0);
}

void screen_cursor(int x, int y)
{
    LCD_Move_Cursor(x * font->width, y * font->height);
}

void screen_get_size(int *w, int *h)
{
    *w = screen_width;
    *h = screen_height;
}

void screen_get_text_size(int *w, int *h)
{
    *w = max_text_width;
    *h = max_text_height;
}

void screen_scroll_up(void)
{
    int i;
    unsigned short *last_str = &screen[font->height * screen_width * (max_text_height - 1)];
    memcpy(&screen[0], &screen[font->height * screen_width], font->height * screen_width * (max_text_height - 1) * 2);
    for (i = 0; i < font->height * screen_width; i++)
	last_str[i] = bg_color;
}
