#include "interrupts.h"

void IRQ_Routine (void) {
    while (1) ;
}

void FIQ_Routine (void)  {
    while (1) ;
}


void SWI_Routine (void)  {
    while (1) ;
}


void UNDEF_Routine (void) {
    while (1) ;
}

void syscall_routine(unsigned long number, unsigned long *regs)
{
    printf("syscall %d (%08X %08X %08X %08X %08X)\n", number, regs[0], regs[1], regs[2], regs[3], regs[4]);
    regs[0] = 0x12345678;
}
