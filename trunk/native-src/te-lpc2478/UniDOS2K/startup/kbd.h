#ifndef _KBD_H_
#define _KBD_H_

#define BACKSPACE   0x08
#define TAB         0x09
#define ENTER       0x0D
#define ESC         0X1B
#define DEL         0X7F

#define UP_ARROW    0xff52
#define DOWN_ARROW  0xff54
#define LEFT_ARROW  0xff51
#define RIGHT_ARROW 0xff53
#define PAGE_UP     0xff55
#define PAGE_DOWN   0xff56
#define HOME        0xff50
#define END         0xff57
#define INSERT      0Xff63
#define F1          0xffbe
#define F2          0xffbf
#define F3          0xffc0
#define F4          0xffc1
#define F5          0xffc2
#define F6          0xffc3
#define F7          0xffc4
#define F8          0xffc5
#define F9          0xffc6
#define F10         0xffc7
#define F11         0xffc8
#define F12         0xffc9

#define SUPER_L     0xffeb
#define SUPER_R     0xffec
#define APPS        0xff65

#define RESEND      0xFE
#define ACK         0xFA
#define BREAKCHAR   0xF0
#define LEFTSHIFT   0x12
#define RIGHTSHIFT  0x59
#define CTRLKEY     0x14
#define ALTKEY      0x11
#define CAPSLOCK    0x58
#define	NUMLOCK     0x77
#define SCROLLLOCK  0x7E
#define DOUBLECODE  0xE1
#define NONUMCODE   0xE0

#define KEYB_KEYUP  0x80000000
#define KEYB_SHIFT  0x40000000
#define KEYB_ALT    0x20000000
#define KEYB_CTRL   0x10000000

int keyboard_init(void);

unsigned int keyboard_get_key(void);

#endif
