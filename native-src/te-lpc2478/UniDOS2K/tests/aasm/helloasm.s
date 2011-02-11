	org	0xA0000000

	entry

	mov	r0, #0x71
	adr	r1, msg
	swi	0x80
	bx	lr

msg	defb	"Hello World!\n", 0
