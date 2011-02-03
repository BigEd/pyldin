#include "screen.h"
#include "console.h"
#include "../kbd.h"

#define CON_TAB_WIDTH 4

static short current_x = 0;
static short current_y = 0;

static int con_width;
static int con_height;

void console_init(void)
{
    current_x = 0;
    current_y = 0;

    screen_get_text_size(&con_width, &con_height);
}

int console_putchar(int c)
{
    c &= 0xff;

    if (c == '\n') {
	current_y++;
	/* current_x = 0; */
    } else if (c == '\r')
	current_x = 0;
    else if (c == '\t')
	current_x += CON_TAB_WIDTH;
    else
	screen_putchar(c, current_x++, current_y);

    if (current_x >= con_width) {
	current_x = 0;
	current_y++;
    }
    if (current_y == con_height) {
	current_y--;
	/* vertical scroll */
	screen_scroll_up();
    }
    return 0;
}

int console_putstring(char *str)
{
    while (*str)
	console_putchar(*str++);
    return 0;
}

int console_getchar(void)
{
    unsigned int c = keyboard_get_key();
    if (!(c & KEYB_KEYUP) && (c & 0xffff))
	return c & 0xff;
    return -1;
}
