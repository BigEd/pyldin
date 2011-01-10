#ifndef _MC6800_H_
#define _MC6800_H_

#define byte	unsigned char
#define word	unsigned short
#define dword	unsigned int

int	mc6800_init(void);
void	mc6800_reset(void);
int	mc6800_step(void);
int	mc6800_fini(void);
void	mc6800_setIrq(int l);
dword	mc6800_get_takts(void);

byte	mc6800_memr(word a);
void	mc6800_memw(word a, byte d);

extern int SWIemulator(int swi, byte *A, byte *B, word *X, byte *t, word *PC);
#endif
