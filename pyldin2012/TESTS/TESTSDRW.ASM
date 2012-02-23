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

	jsr	sdinit

	ldx	#sec1
	jsr	sdreadsec

	ldx	#sec2
	jsr	sdreadsec

;	jsr	sdinit

	ldx	#sec3
	jsr	sdwritesec

stop
	bra	stop

sec1	db	0, 0, 0, 0
	dw	$f000

sec2	db	0, 0, 0, 1
	dw	$f230

sec3	db	0, 0, 0, 2
	dw	$f000

; init SD controller
;
sdinit	proc
	psha
	ldaa	#'i'
	staa	$e6c4
l1
	ldaa	$e6c6
	cmpa	#$ff
	bne	l1
	pula
	rts
	endp

; read sector from SD
;
; X - ptr to structure:
; ds 4 - sector
; ds 2 - buffer ptr
;
sdreadsec proc
	psha
l1
	ldaa	$e6c6
	cmpa	#$ff
	bne	l1

	ldaa	0, x
	staa	$e6c0
	ldaa	1, x
	staa	$e6c1
	ldaa	2, x
	staa	$e6c2
	ldaa	3, x
	staa	$e6c3
	ldx	4, x

	ldaa	#'r'
	staa	$e6c4
l2
	ldaa	$e6c6
	cmpa	#0
	beq	l2
	cmpa	#$ff
	beq l3
	ldaa	$e6c7
	staa	$0,x
	inx
	stx	$E6A2
	bra	l2
l3
	pula
	rts
	endp

; write sector to SD
;
; X - ptr to structure:
; ds 4 - sector
; ds 2 - buffer ptr
;
sdwritesec proc
	psha
l1
	ldaa	$e6c6
	cmpa	#$ff
	bne	l1

	ldaa	0, x
	staa	$e6c0
	ldaa	1, x
	staa	$e6c1
	ldaa	2, x
	staa	$e6c2
	ldaa	3, x
	staa	$e6c3
	ldx	4, x

	ldaa	#'w'
	staa	$e6c4
l2
	ldaa	$e6c6
	cmpa	#0
	beq	l2
	cmpa	#$ff
	beq l3
	ldaa	$0,x
	staa	$e6c7
	inx
	stx	$E6A0
	bra	l2
l3
	pula
	rts
	endp

	ds	$fffe-*, $ff
	dw	start
