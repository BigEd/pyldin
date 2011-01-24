/* ***************************************************************************************************************
crt.s	STARTUP  ASSEMBLY  CODE
	-----------------------
Module includes the interrupt vectors and start-up code.
*************************************************************************************************************** */

		.extern __bss_beg__
		.extern __bss_end__
		.extern __stack_end__
		.extern __data_beg__
		.extern __data_end__
		.extern __data+beg_src__

		.global start
		.global endless_loop

/* Stack Sizes */
.set  UND_STACK_SIZE, 0x00000100		/* stack for "undefined instruction" interrupts is 4 bytes	*/
.set  ABT_STACK_SIZE, 0x00000100		/* stack for "abort" interrupts is 4 bytes			*/
.set  FIQ_STACK_SIZE, 0x00000100		/* stack for "FIQ" interrupts  is 4 bytes			*/
.set  IRQ_STACK_SIZE, 0X00000100		/* stack for "IRQ" normal interrupts is 4 bytes			*/
.set  SVC_STACK_SIZE, 0x00004000		/* stack for "SVC" supervisor mode is 4 bytes			*/

/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs (program status registers) */
.set  MODE_USR, 0x10            		/* Normal User Mode 						*/
.set  MODE_FIQ, 0x11            		/* FIQ Processing Fast Interrupts Mode 				*/
.set  MODE_IRQ, 0x12            		/* IRQ Processing Standard Interrupts Mode 			*/
.set  MODE_SVC, 0x13            		/* Supervisor Processing Software Interrupts Mode 		*/
.set  MODE_ABT, 0x17            		/* Abort Processing memory Faults Mode 				*/
.set  MODE_UND, 0x1B            		/* Undefined Processing Undefined Instructions Mode 		*/
.set  MODE_SYS, 0x1F            		/* System Running Priviledged Operating System Tasks  Mode	*/

.set  I_BIT, 0x80               		/* when I bit is set, IRQ is disabled (program status registers) */
.set  F_BIT, 0x40               		/* when F bit is set, FIQ is disabled (program status registers) */


.section .startup,"ax"
		.code 32
		.align 0

.arm

.global	Reset_Handler
.global _startup
.func   _startup

_startup:

# Exception Vectors

_vectors:       ldr     pc, Reset_Addr
                ldr     pc, Undef_Addr
                ldr     pc, SWI_Addr
                ldr     pc, PAbt_Addr
                ldr     pc, DAbt_Addr
                nop				/* Reserved Vector (holds Philips ISP checksum) */
                ldr     pc, [pc, #-0x120]	/* Interrupt exception for LPC2478 (from VIC)   */
                ldr     pc, FIQ_Addr

Reset_Addr:     .word   Reset_Handler		/* defined in this module below  */
Undef_Addr:     .word   UNDEF_Routine		/* defined in main.c  */
SWI_Addr:       .word   SWI_Handler		/* defined in main.c  */
PAbt_Addr:      .word   UNDEF_Routine		/* defined in main.c  */
DAbt_Addr:      .word   UNDEF_Routine		/* defined in main.c  */
IRQ_Addr:       .word   IRQ_Routine		/* defined in main.c  */
FIQ_Addr:       .word   FIQ_Routine		/* defined in main.c  */
                .word   0			/* rounds the vectors and ISR addresses to 64 bytes total  */


.text
.arm

# Reset Handler

Reset_Handler:
		.extern systemSetup /* Look system.c */
                 ldr     sp, =__stack_end__    @ temporary stack at Stack_Top
                 ldr r0, =systemSetup
                 mov lr, pc
                 bx r0

		/* Setup a stack for each mode - note that this only sets up a usable stack
		 for User mode.   Also each mode is setup with interrupts initially disabled. */
		ldr     r0, =__stack_end__
		msr     CPSR_c, #MODE_UND|I_BIT|F_BIT 	/* Undefined Instruction Mode  */
		mov     sp, r0
		sub     r0, r0, #UND_STACK_SIZE
		msr     CPSR_c, #MODE_ABT|I_BIT|F_BIT 	/* Abort Mode */
		mov     sp, r0
		sub     r0, r0, #ABT_STACK_SIZE
		msr     CPSR_c, #MODE_FIQ|I_BIT|F_BIT 	/* FIQ Mode */
		mov     sp, r0	
		sub     r0, r0, #FIQ_STACK_SIZE
		msr     CPSR_c, #MODE_IRQ|I_BIT|F_BIT 	/* IRQ Mode */
		mov     sp, r0
		sub     r0, r0, #IRQ_STACK_SIZE
		msr     CPSR_c, #MODE_SVC|I_BIT|F_BIT 	/* Supervisor Mode */
		mov     sp, r0
		sub     r0, r0, #SVC_STACK_SIZE
		msr     CPSR_c, #MODE_SYS /*|I_BIT|F_BIT*/ 	/* User Mode */
		mov     sp, r0

		/* copy .data section (Copy from ROM to RAM) */
                ldr     r1, =__data_beg_src__
                ldr     r2, =__data_beg__
                ldr     r3, =__data_end__
1:		cmp     r2, r3
                ldrlo   r0, [r1], #4
                strlo   r0, [r2], #4
                blo     1b

		/* Clear .bss section (Zero init)  */
                mov     r0, #0
                ldr     r1, =__bss_beg__
                ldr     r2, =__bss_end__
2:		cmp     r1, r2
                strlo   r0, [r1], #4
                blo     2b

		/* Init Angel IO */
		/*bl	initialise_monitor_handles */

		/* Enter the C code  */
                b       main

# SWI handler

SWI_Handler:
		sub	sp, sp, #4
		stmfd	sp!, {r0-r12, lr}
		mrs	r2, SPSR
		str	r2, [sp, #14*4]
		mov	r1, sp
		ldr	r0, [lr, #-4]
		bic	r0, r0, #0xff000000
		bl	syscall_routine
		ldr	r2, [sp, #14*4]
		msr	SPSR_csxf, r2
		ldmfd	sp!, {r0-r12, lr}
		add	sp, sp, #4
		movs	pc, lr

.endfunc
.end
