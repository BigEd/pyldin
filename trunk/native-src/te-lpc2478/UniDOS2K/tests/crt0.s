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
		/* Clear .bss section (Zero init)  */
                mov     r3, #0
                ldr     r1, .LC1
                ldr     r2, .LC2
2:		cmp     r1, r2
                strlo   r3, [r1], #4
                blo     2b

                /* Set stack end */
		ldr	r1, =__stack_end__
		str	sp, [r1]

		/* Parse args */

		mov	r1, r0
		mov	r0, #0
		push	{r0}
.LC10:
		ldrb	r3, [r1], #1
		cmp	r3, #0
		beq	.LC12
		cmp	r3, #' '
		beq	.LC10
		cmp	r3, #'"'
		mov	r2, r3
		cmpne	r3, #'\''
		moveq	r2, r3
		movne	r2, #' '
		subne	r1, r1, #1
		push	{r1}
		add	r0, r0, #1
.LC11:
		ldrb	r3, [r1], #1
		cmp	r3, #0
		beq	.LC12
		cmp	r2, r3
		bne	.LC11
		mov	r2, #0
		sub	r3, r1, #1
		strb	r2, [r3]
		b	.LC10
.LC12:
		mov	r1, sp
		add	r2, sp, r0, LSL #2
		mov	r3, sp
.LC13:		cmp	r2, r3
		ldrhi	r4,[r2, #-4]
		ldrhi	r5, [r3]
		strhi	r5, [r2, #-4]!
		strhi	r4, [r3], #4
		bhi	.LC13

		add	r2, sp, r0, LSL #2
		add	r2, r2, #4

		bic	sp, sp, #7

		push	{r2, lr}

		mov	r2, r0
		ldr	r0, .LC3
		bl	setjmp
		cmp	r0, #0
		/* if return from longjmp exit return code */
		ldrne	r0, .LC4
		ldrne	r0, [r0]
		/* else enter the C code  */
		moveq	r0, r2
		bleq	main

		pop	{r2, lr}
		mov	sp, r2

		bx	lr

.LC1:
		.word	__bss_start__
.LC2:
		.word	__bss_end__
.LC3:
		.word	__exit_jump_buf__
.LC4:
		.word	__exit_jump_stat__

		.comm	__stack_end__, 4, 4

		.endfunc
		.end
