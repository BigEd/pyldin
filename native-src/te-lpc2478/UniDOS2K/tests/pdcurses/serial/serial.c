// ==========================================================================
//
//      serial.c
//
//      Public Domain Curses for eCos
//
// ===========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
// ===========================================================================
// ===========================================================================
// #####DESCRIPTIONBEGIN####
//
// Author(s):    Sergei Gavrikov
// Contributors: Sergei Gavrikov
// Date:         2009-04-24
// Purpose:      PDCurses driver for a serial device
// Description:
//
// ####DESCRIPTIONEND####
//
// ========================================================================*/

#include <stdlib.h>                    // free, calloc
#include <unistd.h>
#include <termios.h>

// PDCurses
#include <curspriv.h>
#include <string.h>

#define NDEBUG

# define ENABLE_VT100_ATTR
# define ENABLE_COLOR
//# define ENABLE_KEYPAD

// ---------------------------------------------------------------------------
// acs_map --
//
// A port of PDCurses must provide acs_map[], a 128-element array of chtypes,
// with values laid out based on the Alternate Character Set of the VT100
// (see curses.h). PDC_transform_line() must use this table; when it
// encounters a chtype with the A_ALTCHARSET flag set. For far details reffer
// to a pdcurses/IMPLEMNT document.

chtype          acs_map[128];
// ---------------------------------------------------------------------------

unsigned long   pdc_key_modifiers = 0L;

// ---------------------------------------------------------------------------
// atrtab --
//
// An attibutes table (it is used internally for the port's PDC_init_pair(),
// PDC_pair_content().

static struct {
    short           f,
                    b;
} atrtab[PDC_COLOR_PAIRS];
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PDC_gotoyx --
//
// Move the physical cursor (as opposed to the logical cursor affected by
// wmove()) to the given location. This is called mainly from doupdate(). In
// general, this function need not compare the old location with the new one,
// and should just move the cursor unconditionally.
// ---------------------------------------------------------------------------

void
PDC_gotoyx(int y, int x)
{
    char            buf[8] = { '\033', '[', '0', '0', ';', '0', '0', 'H' };
    unsigned int    len = sizeof(buf);

#ifdef DEBUG
    diag_printf("PDC_gotoyx called: x %d y %d\n", x, y);
#endif

    buf[2] = ((y + 1) / 10) + '0';
    buf[3] = ((y + 1) % 10) + '0';
    buf[5] = ((x + 1) / 10) + '0';
    buf[6] = ((x + 1) % 10) + '0';

    write(1, buf, len);
}

// ---------------------------------------------------------------------------
// PDC_transform_line --
//
// The core output routine. It takes len chtype entities from srcp (a pointer
// into curscr) and renders them to the physical screen at line lineno,
// column x. It must also translate characters 0-127 via acs_map[], if
// they're flagged with A_ALTCHARSET in the attribute portion of the chtype.
// ---------------------------------------------------------------------------

static short    curstoansi[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
static chtype   prev_attr = A_PROTECT;

void
PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    int             i;
    chtype          attr;

#ifdef DEBUG
    diag_printf("PDC_transform_line called: lineno %d curpos %d len %d\n",
                lineno, x, len);
#endif

    PDC_gotoyx(lineno, x);

    for (i = 0; i < len; i++) {
        unsigned char   byte;
        chtype          ch = srcp[i];
        unsigned int    single = 1;

#if defined(ENABLE_COLOR) || defined(ENABLE_VT100_ATTR)
        attr = ch & A_ATTRIBUTES;
        // <ESC>[#; ... m
        if (prev_attr != attr) {
            short           fg,
                            bg,
                            pair;
            char            escape[32];
            unsigned int    count;

            count = 0;
            escape[count++] = '\x1b';
            escape[count++] = '[';
            escape[count++] = '0';
# ifdef ENABLE_COLOR
            pair = PAIR_NUMBER(attr);
            fg = curstoansi[atrtab[pair].f];
            bg = curstoansi[atrtab[pair].b];
            escape[count++] = ';';
            escape[count++] = '3';
            escape[count++] = '0' + fg;
            escape[count++] = ';';
            escape[count++] = '4';
            escape[count++] = '0' + bg;
# endif                         // ENABLE_COLOR
# ifdef ENABLE_VT100_ATTR
            if (attr & A_BOLD) {
                escape[count++] = ';';
                escape[count++] = '1';
            }
            if (attr & A_UNDERLINE) {
                escape[count++] = ';';
                escape[count++] = '4';
            }
            if (attr & A_BLINK) {
                escape[count++] = ';';
                escape[count++] = '5';
            }
            if (attr & A_REVERSE) {
                escape[count++] = ';';
                escape[count++] = '7';
            }
            if (attr & A_INVIS) {
                escape[count++] = ';';
                escape[count++] = '8';
            }
# endif                         // ENABLE_VT100_ATTR
            escape[count++] = 'm';
            write(1, escape, count);

            prev_attr = attr;
        }
#endif                          // ENABLE_COLOR || ENABLE_VT100_ATTR
#ifdef CHTYPE_LONG
        if (ch & A_ALTCHARSET && !(ch & 0xff80))
            ch = acs_map[ch & 0x7f];
#endif
        ch &= 0xff;
        byte = (unsigned char) ch;
        write(1, &byte, single);
    }
}

// ---------------------------------------------------------------------------
// PDC_get_columns --
//
// Returns the size of the screen in columns. It's used in resize_term() to
// set the new value of COLS. (Some existing implementations also call it
// internally from PDC_scr_open(), but this is not required.)
// ---------------------------------------------------------------------------

int
PDC_get_columns(void)
{
    return 64;
}

// ---------------------------------------------------------------------------
// PDC_get_cursor_mode --
//
// Returns the size/shape of the cursor. The format of the result is
// unspecified, except that it must be returned as an int. This function is
// called from initscr(), and the result is stored in SP->orig_cursor, which
// is used by PDC_curs_set() to determine the size/shape of the cursor in
// normal visibility mode (curs_set(1)).
// ---------------------------------------------------------------------------

int
PDC_get_cursor_mode(void)
{
    return 0;
}

// ---------------------------------------------------------------------------
// PDC_get_rows --
//
// Returns the size of the screen in rows. It's used in resize_term() to set
// the new value of LINES. (Some existing implementations also call it
// internally from PDC_scr_open(), but this is not required.)
// ---------------------------------------------------------------------------

int
PDC_get_rows(void)
{
    return 29;
}

// ---------------------------------------------------------------------------
// get key and check for ESC sequence
// ---------------------------------------------------------------------------

#define ESC_SEQ_LEN	32

static int read_key_and_decode(void)
{
    static unsigned char buf[ESC_SEQ_LEN];
    static int esc_seq = 0;
    static int esc_run = 0;
    unsigned char c = 0;

    if (esc_run) {
	if (esc_seq < esc_run) {
	    return buf[esc_seq++];
	}
	esc_run = 0;
	esc_seq = 0;
    }

    if (read(0, &c, 1) != 1)
	return -1;

    if (esc_seq == 1) {
	buf[esc_seq++] = c;
	if (c == '[') {
	    return -1;
	}
	esc_run = esc_seq;
	esc_seq = 0;
	return -1;
    } else if (esc_seq) {
	if ((c >= '0' && c <= '9') && esc_seq < ESC_SEQ_LEN - 1) {
	    buf[esc_seq++] = c;
	    return -1;
	}
#if 0
	 else if (esc_seq > ESC_SEQ_LEN) {
	    esc_run = esc_seq;
	    esc_seq = 0;
	    return -1;
	}
#endif
	buf[esc_seq++] = c;
	if (esc_seq == 3) {
	    if (buf[2] == 'A') { esc_seq = 0; return KEY_UP; }
	    if (buf[2] == 'B') { esc_seq = 0; return KEY_DOWN; }
	    if (buf[2] == 'C') { esc_seq = 0; return KEY_RIGHT; }
	    if (buf[2] == 'D') { esc_seq = 0; return KEY_LEFT; }
	    if (buf[2] == 'O') { return -1; }
	}
	if (esc_seq == 4) {
	    if (buf[3] == '~') {
		if (buf[2] == '2') { esc_seq = 0; return KEY_IC; }
		if (buf[2] == '3') { esc_seq = 0; return KEY_DC; }
		if (buf[2] == '5') { esc_seq = 0; return KEY_PPAGE; }
		if (buf[2] == '6') { esc_seq = 0; return KEY_NPAGE; }
	    }
	    if (buf[2] == 'O') {
		if (buf[3] == 'H') { esc_seq = 0; return KEY_HOME; }
		if (buf[3] == 'F') { esc_seq = 0; return KEY_END; }
		if (buf[3] == 'P') { esc_seq = 0; return KEY_F(1); }
		if (buf[3] == 'Q') { esc_seq = 0; return KEY_F(2); }
		if (buf[3] == 'R') { esc_seq = 0; return KEY_F(3); }
		if (buf[3] == 'S') { esc_seq = 0; return KEY_F(4); }
	    }
	}
	esc_run = esc_seq;
	esc_seq = 0;
	return -1;
    }

    if (c == 0x1b) {
	buf[esc_seq++] = c;
	return -1;
    }

    return c;
}

// ---------------------------------------------------------------------------
// PDC_check_key --
//
// Keyboard/mouse event check, called from wgetch(). Returns TRUE if there's
// an event ready to process. This function must be non-blocking.
// ---------------------------------------------------------------------------

static int unget_c = -1;

bool
PDC_check_key(void)
{
    int c = 0;
    if ((c = read_key_and_decode()) != -1) {
	unget_c = c;
	return TRUE;
    }
    unget_c = -1;
    return FALSE;
}

// ---------------------------------------------------------------------------
// PDC_flushinp --
//
// This is the core of flushinp(). It discards any pending key or mouse
// events, removing them from any internal queue and from the OS queue, if
// applicable.
// ---------------------------------------------------------------------------

void
PDC_flushinp(void)
{
}

// ---------------------------------------------------------------------------
// PDC_get_key --
//
// Get the next available key, or mouse event (indicated by a return of
// KEY_MOUSE), and remove it from the OS' input queue, if applicable. This
// function is called from wgetch(). This function may be blocking, and
// traditionally is; but it need not be. If a valid key or mouse event cannot
// be returned, for any reason, this function returns -1. For more details
// read `pdcurses/IMPLEMNT' document.
// ---------------------------------------------------------------------------

int
PDC_get_key(void)
{
    int c = 0;
    if (unget_c != -1) {
	c = unget_c;
	unget_c = -1;
	return c;
    }
//    while ((c = read_key_and_decode()) == -1);
//    return c;
    return -1;
}

// ---------------------------------------------------------------------------
// PDC_modifiers_set --
//
// Called from PDC_return_key_modifiers(). If your platform needs to do
// anything in response to a change in SP->return_key_modifiers, do it here.
// Returns OK or ERR, which is passed on by the caller.
// ---------------------------------------------------------------------------

int
PDC_modifiers_set(void)
{
    return ERR;
}

// ---------------------------------------------------------------------------
// PDC_mouse_set --
//
// Called by mouse_set(), mouse_on(), and mouse_off() -- all the functions
// that modify SP->_trap_mbe. If your platform needs to do anything in
// response to a change in SP->_trap_mbe (for example, turning the mouse
// cursor on or off), do it here. Returns OK or ERR, which is passed on by
// the caller.
// ---------------------------------------------------------------------------

int
PDC_mouse_set(void)
{
    return ERR;
}

// ---------------------------------------------------------------------------
// PDC_set_keyboard_binary --
//
// Set keyboard input to "binary" mode. If you need to do something to keep
// the OS from processing ^C, etc. on your platform, do it here. TRUE turns
// the mode on; FALSE reverts it. This function is called from raw() and
// noraw().
// ---------------------------------------------------------------------------

void
PDC_set_keyboard_binary(bool on)
{
    struct termios raw;
#ifdef DEBUF
    diag_printf("PDC_set_keyboard_binary() called: on %d\n", on);
#endif

    if (tcgetattr(0, &raw) != -1) {
	if (on) {
	    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	    raw.c_cc[VMIN] = 0;
	    raw.c_cc[VTIME] = 0;
	} else {
	    raw.c_lflag |=  (ECHO | ICANON | IEXTEN | ISIG);
	    raw.c_cc[VMIN] = 1;
	    raw.c_cc[VTIME] = 0;
	}
	if (tcsetattr(0, TCSAFLUSH, &raw) != -1)
	    return;
    }
#ifdef DEBUF
    diag_printf("PDC_set_keyboard_binary() called: error\n");
#endif
}

// ---------------------------------------------------------------------------
// PDC_can_change_color --
//
// Returns TRUE if init_color() and color_content() give meaningful results,
// FALSE otherwise. Called from can_change_color().
// ---------------------------------------------------------------------------

bool
PDC_can_change_color(void)
{
    return FALSE;
}

// ---------------------------------------------------------------------------
// PDC_color_content --
//
// The core of color_content(). This does all the work of that function,
// except checking for values out of range and null pointers.
// ---------------------------------------------------------------------------

int
PDC_color_content(short color, short *red, short *green, short *blue)
{
    return ERR;
}

// ---------------------------------------------------------------------------
// PDC_init_color --
//
// The core of init_color(). This does all the work of that function, except
// checking for values out of range.
// ---------------------------------------------------------------------------

int
PDC_init_color(short color, short red, short green, short blue)
{
    return ERR;
}

// ---------------------------------------------------------------------------
// PDC_init_pair --
//
// The core of init_pair(). This does all the work of that function, except
// checking for values out of range. The values passed to this function
// should be returned by a call to PDC_pair_content() with the same pair
// number. PDC_transform_line() should use the specified colors when
// rendering a chtype with the given pair number.
// ---------------------------------------------------------------------------

void
PDC_init_pair(short pair, short fg, short bg)
{
#ifdef DEBUG
    diag_printf("PDC_init_pair called: pair %d fg %d bg %d\n", pair, fg,
                bg);
#endif

    atrtab[pair].f = fg;
    atrtab[pair].b = bg;
}

// ---------------------------------------------------------------------------
// PDC_pair_content --
//
// The core of pair_content(). This does all the work of that function,
// except checking for values out of range and null pointers.
// ---------------------------------------------------------------------------

int
PDC_pair_content(short pair, short *fg, short *bg)
{
    *fg = atrtab[pair].f;
    *bg = atrtab[pair].b;

    return OK;
}

// ---------------------------------------------------------------------------
// PDC_reset_prog_mode --
//
// The non-portable functionality of reset_prog_mode() is handled here --
// whatever's not done in _restore_mode(). In current ports: In OS/2, this
// sets the keyboard to binary mode; in Win32, it enables or disables the
// mouse pointer to match the saved mode; in others it does nothing.
// ---------------------------------------------------------------------------

void
PDC_reset_prog_mode(void)
{
}

// ---------------------------------------------------------------------------
// PDC_reset_shell_mode --
//
// The same thing, for reset_shell_mode(). In OS/2 and Win32, it restores the
// default console mode; in others it does nothing.
// ---------------------------------------------------------------------------

void
PDC_reset_shell_mode(void)
{
}

// ---------------------------------------------------------------------------
// PDC_resize_screen --
//
// This does the main work of resize_term(). It may respond to non-zero
// parameters, by setting the screen to the specified size; to zero
// parameters, by setting the screen to a size chosen by the user at runtime,
// in an unspecified way (e.g., by dragging the edges of the window); or
// both. It may also do nothing, if there's no appropriate action for the
// platform.
// ---------------------------------------------------------------------------

int
PDC_resize_screen(int nlines, int ncols)
{
    if (nlines == 0 && ncols == 0)
        return OK;

    SP->resized = FALSE;
    SP->cursrow = SP->curscol = 0;

    return OK;

}

// ---------------------------------------------------------------------------
// PDC_restore_screen_mode --
//
// Called from _restore_mode() in kernel.c, this function does the actual
// mode changing, if applicable. Currently used only in DOS and OS/2.
// ---------------------------------------------------------------------------

void
PDC_restore_screen_mode(int i)
{
}

// ---------------------------------------------------------------------------
// PDC_save_screen_mode --
//
// Called from _save_mode() in kernel.c, this function saves the actual
// screen mode, if applicable. Currently used only in DOS and OS/2.
// ---------------------------------------------------------------------------

void
PDC_save_screen_mode(int i)
{
}

// ---------------------------------------------------------------------------
// PDC_scr_close --
//
// The platform-specific part of endwin(). It may restore the image of the
// original screen saved by PDC_scr_open(), if the PDC_RESTORE_SCREEN
// environment variable is set; either way, if using an existing terminal,
// this function should restore it to the mode it had at startup, and move
// the cursor to the lower left corner. (The X11 port does nothing.)
// ---------------------------------------------------------------------------

void
PDC_scr_close(void)
{
}

// ---------------------------------------------------------------------------
// PDC_scr_free --
//
// Frees the memory for SP allocated by PDC_scr_open(). Called by
// delscreen().
// ---------------------------------------------------------------------------

void
PDC_scr_free(void)
{
#ifdef DEBUG
    diag_printf("PDC_scr_free called\n");
#endif

    PDC_set_keyboard_binary(FALSE);

    if (SP)
        free(SP);
}

// ---------------------------------------------------------------------------
// PDC_scr_open --
//
// The platform-specific part of initscr(). It's actually called from
// Xinitscr(); the arguments, if present, correspond to those used with
// main(), and may be used to set the title of the terminal window, or for
// other, platform-specific purposes.
// ---------------------------------------------------------------------------

int
PDC_scr_open(int argc, char **argv)
{
    static bool     acs_map_init = FALSE;

    int             i = 0;

#ifdef DEBUG
    diag_printf("PDC_scr_open called\n");
#endif

    SP = calloc(1, sizeof(SCREEN));

    if (!SP)
        return ERR;

    SP->cols = PDC_get_columns();
    SP->lines = PDC_get_rows();
    SP->cursrow = SP->curscol = 0;
    SP->raw_inp = SP->raw_out = TRUE;

    if (!acs_map_init)
        for (i = 0; i < 128; i++) {
            // Square shapes, no more
            switch (i) {
            case 'j':
            case 'k':
            case 'l':
            case 'm':
                acs_map[i] = '+';
                break;
            case 'q':
                acs_map[i] = '-';
                break;
            case 'x':
                acs_map[i] = '|';
                break;
            default:
                acs_map[i] = (chtype) i | A_ALTCHARSET;
            }
        }

    PDC_set_keyboard_binary(TRUE);

    return OK;
}

// ---------------------------------------------------------------------------
// PDC_curs_set --
//
// Called from curs_set(). Changes the appearance of the cursor -- 0 turns it
// off, 1 is normal (the terminal's default, if applicable, as determined by
// SP->orig_cursor), and 2 is high visibility. The exact appearance of these
// modes is not specified.
// ---------------------------------------------------------------------------

int
PDC_curs_set(int visibility)
{
    int             vis;
    char            buf[6] = { '\033', '[', '?', '2', '5', 'h' };
    unsigned int    len = sizeof(buf);

#ifdef DEBUG
    diag_printf("PDC_curs_set called: visibility %d\n", visibility);
#endif

    vis = SP->visibility;

    if (visibility)
        // <ESC>[?25h -- show cursor
        buf[5] = 'h';
    else
        // <ESC>[?25l -- hide cursor
        buf[5] = 'l';

//    write(1, buf, &len);

    SP->visibility = vis;

    return vis;
}

// ---------------------------------------------------------------------------
// PDC_beep --
//
// Emits a short audible beep. If this is not possible on your platform, you
// must set SP->audible to FALSE during initialization (i.e., from
// PDC_scr_open() -- not here); otherwise, set it to TRUE. This function is
// called from beep().
// ---------------------------------------------------------------------------

void
PDC_beep(void)
{
}

// ---------------------------------------------------------------------------
// PDC_napms --
//
// This is the core delay routine, called by napms(). It pauses for about
// (the X/Open spec says "at least") ms milliseconds, then returns. High
// degrees of accuracy and precision are not expected (though desirable, if
// you can achieve them). More important is that this function gives back the
// process' time slice to the OS, so that PDCurses idles at low CPU usage.
// ---------------------------------------------------------------------------

void
PDC_napms(int ms)
{
#warning "Delay need"
    // There is alone call of napms() in current PDCurses implementation, its
    // aragument is 50 (50 of 1/20th second ticks).

//    cyg_thread_delay(ms / 10);
//    usleep(ms * 100);
}

// ---------------------------------------------------------------------------
// PDC_sysname --
//
// Returns a short string describing the platform, such as "DOS" or "X11".
// This is used by longname(). It must be no more than 100 characters; it
// should be much, much shorter (existing platforms use no more than 5).
// ---------------------------------------------------------------------------

const char     *
PDC_sysname(void)
{
    return "UniDOS";
}

// ---------------------------------------------------------------------------
// PDC_getclipboard, PDC_clearclipboard, PDC_freeclipboard PDC_setclipboard,
// PDC_get_input_fd, PDC_set_blink, PDC_set_title --
//
// The following functions are implemented in the platform directories, but
// are accessed directly by apps. Refer to the user documentation for their
// descriptions:
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PDC_getclipboard --
//
int
PDC_getclipboard(char **contents, long *length)
{
    return PDC_CLIP_ACCESS_ERROR;
}

// ---------------------------------------------------------------------------
// PDC_clearclipboard --
//
int
PDC_clearclipboard(void)
{
    return PDC_CLIP_ACCESS_ERROR;
}

// ---------------------------------------------------------------------------
// PDC_freeclipboard --
//
int
PDC_freeclipboard(char *contents)
{
    return PDC_CLIP_ACCESS_ERROR;
}

// ---------------------------------------------------------------------------
// PDC_setclipboard --
//
int
PDC_setclipboard(const char *contents, long length)
{
    return PDC_CLIP_ACCESS_ERROR;
}

// ---------------------------------------------------------------------------
// PDC_get_input_fd --
//
unsigned long
PDC_get_input_fd(void)
{
    return -1;
}

// ---------------------------------------------------------------------------
// PDC_set_blink --
//
int
PDC_set_blink(bool blinkon)
{
#ifdef DEBUG
    diag_printf("PDC_set_blink called: blinkon %d\n", blinkon);
#endif

    // start_color() guides that PDC_set_blink() should also set COLORS, to 8
    // or 16.
#ifdef ENABLE_COLOR
    COLORS = 8;
#else
    COLORS = 0;
#endif

    return blinkon ? ERR : OK;
}

// ---------------------------------------------------------------------------
// PDC_set_title --
//
void
PDC_set_title(const char *title)
{
}

// ---------------------------------------------------------------------------
// EOF serial.c
