/* ***************************************************************************************************************
crt0.s	STARTUP  ASSEMBLY  CODE
	-----------------------
Module includes the interrupt vectors and start-up code.
*************************************************************************************************************** */

		.code 32
		.align 0

		.text
		.arm

		.global _start
		.func   _start

_start:
		push	{r0, r1, r2}

		/* Clear .bss section (Zero init)  */
                mov     r0, #0
                ldr     r1, .LC1
                ldr     r2, .LC2
2:		cmp     r1, r2
                strlo   r0, [r1], #4
                blo     2b

		pop	{r0, r1, r2}

                /* Set stack end */
		ldr	r3, =__stack_end__
		str	sp, [r3]
		push	{lr}

		/* Enter the C code  */
                bl       main

		pop	{lr}
		mov	pc, lr

.LC1:
		.word	__bss_start__
.LC2:
		.word	__bss_end__

		.comm	__stack_end__, 4, 4

		.endfunc
		.end
