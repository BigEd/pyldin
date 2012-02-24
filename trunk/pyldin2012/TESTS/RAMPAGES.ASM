rampage		equ	$e6f0
rampagelock	equ	$e6f1

	org	$100

	clr	rampagelock

	ldab	#1
	ldaa	#0

	staa	rampage
	stab	$c000

	ldaa	#8
	incb

	staa	rampage
	stab	$c000

	inca
	incb

	staa	rampage
	stab	$c000

	inca
	incb

	staa	rampage
	stab	$c000

	ldaa	#0
	staa	rampage
	ldab	$c000
	stab	$e6a0

	ldaa	#8
	staa	rampage
	ldab	$c000
	stab	$e6a1

	inca
	staa	rampage
	ldab	$c000
	stab	$e6a2

	inca
	staa	rampage
	ldab	$c000
	stab	$e6a3

	rts
