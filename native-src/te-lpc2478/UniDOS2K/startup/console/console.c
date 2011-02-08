#include "screen.h"
#include "console.h"
#include "../kbd.h"

#define _isdigit(c) ((c >= '0' && c <= '9')?1:0)

#define CON_TAB_WIDTH 4

static short con_x = 0;
static short con_y = 0;

static int con_width;
static int con_height;

static int con_col_fg;
static int con_col_bg;

#define MAKECOLOR(a, b, c) (((a >> 3) << 10) | ((b >> 3) << 5) | (c >> 3))

enum {
    COL_BLACK = 0,
    COL_RED,
    COL_GREEN,
    COL_YELLOW,
    COL_BLUE,
    COL_PURPLE,
    COL_CYAN,
    COL_WHITE
};

static unsigned short colors[8] = {
    MAKECOLOR(0x00,0x00,0x00),
    MAKECOLOR(0xaa,0x00,0x00),
    MAKECOLOR(0x00,0xaa,0x00),
    MAKECOLOR(0xaa,0x55,0x00),
    MAKECOLOR(0x00,0x00,0xaa),
    MAKECOLOR(0xaa,0x00,0xaa),
    MAKECOLOR(0x00,0xaa,0xaa),
    MAKECOLOR(0xaa,0xaa,0xaa),
};

void console_init(void)
{
    con_x = 0;
    con_y = 0;

    con_col_fg = colors[COL_WHITE];
    con_col_bg = colors[COL_BLACK];

    screen_fgcolor(con_col_fg);
    screen_bgcolor(con_col_bg);

    screen_get_text_size(&con_width, &con_height);
}

#define getcolor(a) (((a % 10) > 7)?7:(a % 10))

static void set_attributes(int c, int *arg)
{
    while (c) {
	int a = arg[c];
	if (a >= 40)
	    screen_bgcolor(colors[getcolor(a)]);
	else if (a >= 30)
	    screen_fgcolor(colors[getcolor(a)]);
	else {
	    switch (a) {
	    case 0:
		screen_fgcolor(con_col_fg);
		screen_bgcolor(con_col_bg);
		break;
	    case 7:
		screen_fgcolor(con_col_bg);
		screen_bgcolor(con_col_fg);
		break;
	    case 8:
		screen_fgcolor(con_col_bg);
		screen_bgcolor(con_col_bg);
		break;
	    }
	}
	c--;
    }
}

int console_putchar(int c)
{
    static short saved_x = 0;
    static short saved_y = 0;
    static int esc_seq = 0;
    static int esc_num = 0;
    static int esc_arg[9];

    c &= 0xff;

    if (esc_seq == 1) {
	if (c == '[') {
	    esc_seq++;
	    esc_num = 0;
	} else
	    esc_seq = 0;
	return 0;
    } else if (esc_seq) {
	if (_isdigit(c)) {
	    if (!esc_num) {
		esc_num = (esc_num + 1) % 8;
		if (esc_num == 0) esc_num++;
		esc_arg[esc_num] = 0;
	    }
	    esc_arg[esc_num] = esc_arg[esc_num] * 10 + (c - '0');
	    return 0;
	} else {
	    if (c == ';') {
		esc_num = (esc_num + 1) % 8;
		if (esc_num == 0) esc_num++;
		esc_arg[esc_num] = 0;
		return 0;
	    }
	    switch (c) {
	    case 'm':
		set_attributes(esc_num, esc_arg);
		break;
	    case 'H':
	    case 'f':
		if (esc_num > 1) {
		    con_y = esc_arg[1];
		    con_x = esc_arg[2];
		}
		break;
	    case 'A':
		if (esc_num >= 1)
		    con_y -= esc_arg[1];
		break;
	    case 'B':
		if (esc_num >= 1)
		    con_y += esc_arg[1];
		break;
	    case 'C':
		if (esc_num >= 1)
		    con_x += esc_arg[1];
		break;
	    case 'D':
		if (esc_num >= 1)
		    con_x -= esc_arg[1];
		break;
	    case 'J':
		if (esc_num >= 1) {
		    con_x = 0;
		    con_y = 0;
		    screen_clear();
		}
		break;
	    case 'K':
		break;
	    case 's':
		saved_x = con_x;
		saved_y = con_y;
		break;
	    case 'u':
		con_x = saved_x;
		con_y = saved_y;
		break;
	    }
	    esc_seq = 0;
	}
	if (con_x < 0) con_x = 0;
	if (con_x >= con_width) con_x = con_width - 1;
	if (con_y < 0) con_y = 0;
	if (con_y >= con_height) con_y = con_height - 1;
    } else {
	if (c == '\n') {
	    con_y++;
	    /* con_x = 0; */
	} else if (c == '\r')
	    con_x = 0;
	else if (c == '\t')
	    con_x += CON_TAB_WIDTH;
	else if (c == 0x1b) {
	    esc_seq = 1;
	    return 0;
	} else
	    screen_putchar(c, con_x++, con_y);
    }

    if (con_x >= con_width) {
	con_x = 0;
	con_y++;
    }
    if (con_y == con_height) {
	con_y--;
	/* vertical scroll */
	screen_scroll_up();
    }
    screen_cursor(con_x, con_y);
    return 0;
}

int console_putstring(char *str)
{
    while (*str)
	console_putchar(*str++);
    return 0;
}

#ifdef USE_KBD
int console_getchar(void)
{
    unsigned int c = keyboard_get_key();
    if (!(c & KEYB_KEYUP) && (c & 0xffff))
	return c & 0xff;
    return -1;
}
#endif
