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

                /* Set stack end */
		ldr	r1, =__stack_end__
		mov	r2, sp
		add	r2, r2, #12
		str	r2, [r1]

		pop	{r0, r1, r2}

		/* Enter the C code  */
		push	{lr}
                bl       main
		pop	{lr}

		bx	lr

.LC1:
		.word	__bss_start__
.LC2:
		.word	__bss_end__

		.comm	__stack_end__, 4, 4

		.endfunc
		.end
