#include "screen.h"
#include "console.h"
#include "../kbd.h"

#define CON_TAB_WIDTH 4

static short con_x = 0;
static short con_y = 0;

static int con_width;
static int con_height;

void console_init(void)
{
    con_x = 0;
    con_y = 0;

    screen_get_text_size(&con_width, &con_height);
}

int console_putchar(int c)
{
    c &= 0xff;

    if (c == '\n') {
	con_y++;
	/* con_x = 0; */
    } else if (c == '\r')
	con_x = 0;
    else if (c == '\t')
	con_x += CON_TAB_WIDTH;
    else
	screen_putchar(c, con_x++, con_y);

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
