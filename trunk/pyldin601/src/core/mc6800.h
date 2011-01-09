#ifndef _MC6800_H_
#define _MC6800_H_

#define byte	unsigned char
#define word	unsigned short
#define dword	unsigned int

extern dword	mc6800_global_takts;
extern int 	mc6800_init(void);
extern byte 	*mc6800_getRomPtr(int chip, int page);
extern void 	mc6800_setDATETIME(word year, word mon, word mday, word hour, word min, word sec);
extern void 	mc6800_reset(void);
extern int 	mc6800_step(void);
extern int 	mc6800_fini(void);

extern byte mc6800_memr(word a);
extern void mc6800_memw(word a, byte d);

extern int SWIemulator(int swi, byte *A, byte *B, word *X, byte *t, word *PC);
#endif
