#ifndef _PRINTER_H_
#define _PRINTER_H_

#define PRINTER_NONE	0
#define PRINTER_FILE	1
#define PRINTER_SYSTEM	2
#define PRINTER_COVOX	3

int printer_init(int mode);
int printer_fini(void);

int printer_dra_wr(unsigned char data);
int printer_drb_wr(unsigned char data);
int printer_cra_wr(unsigned char data);
int printer_crb_wr(unsigned char data);

unsigned char printer_dra_rd(void);
unsigned char printer_drb_rd(void);
unsigned char printer_cra_rd(void);
unsigned char printer_crb_rd(void);

extern void printer_put_char(byte data);
extern void Covox_Set(int val, int ticks);

#endif
