	org	$f000

start
	ldaa	#$0c
	staa	$e600
	ldab	#$f0
	stab	$e601
	inca
	staa	$e600
	ldab	#$00
	stab	$e601

	ldx	#$f000
loop0
	clr	0,x
	inx
	cpx	#$F3E8
	bne	loop0


	ldx	#$f000
	lds	#text
loop
	pula
	staa	0,x
	inx
	cmpa	#0
	bne	loop
	nop
	ldaa	#'#'
	staa	$0

	nop

testram
	ldaa	#$00
	staa	$e6a0
	staa	$e6a1
	staa	$e6a2
	staa	$e6a3
testram1
	ldx	#$0000
	staa	$e6a3
testram2
	stx	$e6a0
	staa	0,x
	inx
	cpx	#$E000
	bne	testram2
	nop

	ldx	#$f028
testram22
	staa	0,x
	inx
	cpx	#$0000
	bne	testram22

	nop
	ldx	#$0000
testram3
	stx	$e6a0
	ldab	0,x
	stab	$e6a2
	cba
	bne	stop
	inx
	cpx	#$E000
	bne	testram3
	inca
	bra testram1
	nop
stop
	bra	stop


text
	db	'+Wellcome to Pyldin 2012!',0

	ds	$fffe-*, $ff
	dw	start
