#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "config.h"

void IRQ_Routine (void)   __attribute__ ((interrupt("IRQ")));
void FIQ_Routine (void)   __attribute__ ((interrupt("FIQ")));
void SWI_Routine (void)   __attribute__ ((interrupt("SWI")));
void UNDEF_Routine (void) __attribute__ ((interrupt("UNDEF")));


#endif /* _INTERRUPTS_H_ */


