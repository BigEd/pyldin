fname		=	$AAAA

		org	$101
		bvs	start

		org	$150
start

		ldx	#fname
		stx	table
		ldx	#0
		stx	table+2
		ldaa	#1
		ldx	#table
		int	$4A
		tsta
		bne	error
		stab	handle

		tba
		ldx	#size_buff
		int	$51

		ldx	size_buff
		bne	error
		ldx	size_buff+2
		stx	table+2
		ldx	#$1000
		stx	table
		ldaa	handle
		ldx	#table
		int	$4C
		tsta
		bne	error
		cpx	size_buff+2
		bne	error

		ldaa	handle
		int	$4E
		jmp	$1000
error
		ldx	#$260
		int	$23
		int	$20
		clr	$E6C0
		ldx	$FFFE
		stx	$EE00
		jmp	x

table		ds	4
size_buff	ds	4
handle		ds	1
		end
