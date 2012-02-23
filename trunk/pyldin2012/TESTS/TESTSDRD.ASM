	org	$f000

start
	sei
	lds	#$1000

	ldaa	#$0c
	staa	$e600
	ldab	#$f0
	stab	$e601
	inca
	staa	$e600
	ldab	#$00
	stab	$e601

	ldx	#$0
	stx	$e6a0
	stx	$e6a2

l1
	ldaa	$e6c6
	cmpa	#0
	beq	l1

	ldaa	#0
	staa	$E6C0
	staa	$E6C1
	staa	$E6C2
	staa	$E6C3

	ldx	#$f000
	stx	$E6A0
	stx	$E6A2

	staa	$E6C4

l2
	ldaa	$e6c6
	cmpa	#0
	beq	l2
	cmpa	#$ff
	beq lout

	ldaa	$E6C7
	staa	$E6A1
	staa	$0,x
	inx
	stx	$E6A2
	cpx	#$f400
	beq	lout
	bra	l2

lout
	stx	$E6A2

stop
	bra	stop


	ds	$fffe-*, $ff
	dw	start
