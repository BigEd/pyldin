rampage		equ	$e6f0
rampagelock	equ	$e6f1

secs		equ	$200
page		equ	$206

		org	$100

		clr	rampagelock
		ldaa	#8
		staa	page

		ldx	parts

		ldaa	$b,x
		staa	secs+0
		ldaa	$a,x
		staa	secs+1
		ldaa	9,x
		staa	secs+2
		ldaa	8,x
		staa	secs+3

lpages
		ldaa	page
		cmpa	#$10
		beq	exit
		staa	rampage

		ldx	#$c000
		stx	secs+4

		clrb
		ldx	#secs
lpage		ldaa	#$41
		int	$17

		inc	4,x	; increase buffer ptr +$200
		inc	4,x	;

		ldaa	3,x	; increase sector count
		adda	#1
		staa	3,x
		ldaa	2,x
		adca	#0
		staa	2,x
		ldaa	1,x
		adca	#0
		staa	1,x
		ldaa	0,x
		adca	#0
		staa	0,x

		incb
		cmpb	#16
		bne	lpage

		inc	page
		bra	lpages
exit		ldaa	#$ff
		staa	rampagelock
		rts

parts	dw	$2BE, $2CE, $2DE, $2EE
