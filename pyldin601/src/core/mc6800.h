#ifndef _MC6800_H_
#define _MC6800_H_

#define byte	unsigned char
#define word	unsigned short
#define dword	unsigned int

#ifndef O_INLINE
#define O_INLINE
#endif

int	mc6800_init(void);
void	mc6800_reset(void);
int	mc6800_step(void);
int	mc6800_fini(void);

O_INLINE void	mc6800_setIrq(int l);
O_INLINE dword	mc6800_get_takts(void);
O_INLINE byte 	*mc6800_get_memory(void);

O_INLINE byte mc6800_memr(word a);
O_INLINE void mc6800_memw(word a, byte d);

extern byte *get_cpu_mem(dword size);
#endif
