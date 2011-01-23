#ifndef _KBD_H_
#define _KBD_H_

#define KEY_SHIFT	0x60
#define KEY_CONTROL	0x61
#define KEY_PAUSE	0x6f

int keyboard_init(void);

extern void keyboard_handler(uint8_t scancode);

#endif
