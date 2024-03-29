#ifndef _DEVICES_H_
#define _DEVICES_H_

#define MAX_ROMCHIPS	5

/*
 * LED bits
 */

#define KBD_LED_SCROLL     0x01
#define KBD_LED_NUM        0x02
#define KBD_LED_CAPS       0x04

int devices_init(void);
int devices_fini(void);
void devices_reset(void);
void devices_setDATETIME(word year, word mon, word mday, word hour, word min, word sec);

O_INLINE void devices_set_tick50(void);

O_INLINE int devices_memr(word a, byte *t);
O_INLINE int devices_memw(word a, byte d);

extern byte *get_bios_mem(dword size);
extern byte *get_ramdisk_mem(dword size);
extern byte *get_romchip_mem(byte chip, dword size);

#endif
