#include "interrupts.h"
#include "uart.h"

#define hex4(a) (((a & 0xf) <= 9)?((a & 0xf) + '0'):((a & 0xf) + 'W'))

#define hex(val) { \
    int i; \
    for (i = 0; i < 8; i++) { \
	uart0Putch(hex4((val) >> ((7 - i) * 4))); \
    } \
}

#define SHOWLR(MSG) { \
    register volatile unsigned long val;	\
    asm volatile ("mov %0, lr": "=r" (val));	\
    uart0Puts( MSG "\r\nLR=[");			\
    hex(val);					\
    uart0Puts("] ");				\
}

#define SHOWMSG(MSG) uart0Puts(MSG);

void IRQ_Routine (void) {
    SHOWMSG("IRQ\r\n");
    while (1) ;
}

void FIQ_Routine (void)  {
    SHOWMSG("FIQ\r\n");
    while (1) ;
}


void SWI_Routine (void)  {
    SHOWMSG("SWI\r\n");
    while (1) ;
}

void UNDEF_Routine (void) {
    SHOWLR("UNDEF");
    while (1) ;
}

void PAbt_Routine (void) {
    SHOWLR("Prefetch Abort");
    while (1) ;
}

void DAbt_Routine (void) {
    SHOWLR("Data Abort");
    while (1) ;
}
