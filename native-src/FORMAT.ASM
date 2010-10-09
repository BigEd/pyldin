
;   Format utility for Pyldin 601	   Version 2.0
;
;  (C) Ivan Gorinov   1989

; constants

max_retry	=	2
my_fill_byte	=	$E5
s_per_clust_off =	$0D		; offset of sectors per cluster in boot
root_size_off	=	$11		; offset of root dir size in boot
totalsec_off	=	$13		; offset of total sectors in boot
mediadesc_off	=	$15		; offset of media descriptor in boot
fat_size_off	=	$16		; offset of FAT size in boot
heads_off	=	$1A		; offset of number of heads in boot

; error code from int $17

;			bit 0 - write protected
;			bit 1 - overrun
;			bit 2 - bad CRC
;			bit 3 - bad record
;			bit 4 - bad address field
;			bit 5 - seek error
;			bit 6 - timeout
;			bit 7 - bad FDC

; Interrupt vectors

break		=	$04
wait_key	=	$10
floppy_disk	=	$17
getchar 	=	$20
getline 	=	$21
print_char	=	$22
print_string	=	$23
print_dec	=	$24
print_hex	=	$25
mul_A_B 	=	$28
div_X_BA	=	$29
allocate	=	$2A
deallocate	=	$2B
move		=	$2D
get_vector	=	$2E
set_vector	=	$2F
get_free_mem	=	$36
chain		=	$37
terminate	=	$38
get_paramcount	=	$3B
get_paramstr	=	$3C
logical_sector	=	$40
get_dos_version =	$41
get_cur_drive	=	$45
get_free_space	=	$52

; system variables usage

drive_present	=	$ED02
drive_type	=	$ED03
fdd_parms	=	$ED26

		jmp	format

dos_version	db	2, 0
		db	'Format Utility    Version 2.05',0

copyright	db	'(C) çàèã "è∞Æ£∞†¨≠Æ Æ±®£≥∞ø¢†≠•"  1988-89',0
		db	'(C) à¢†≠ ÉÆ∞®≠Æ¢      1989',0

int17_table			       ; used for all i/o oprations
drive		ds	1	       ; in all procedures
track		ds	1
head		ds	1
sector		ds	1
buffer		ds	2

is_80		db	0		; if<>0 then 80 tracks
max_track	db	39
max_head	db	1
sec_per_track	db	9
double_step	db	0		; if=$80 then double stepping needed
work_buffer	ds	2		; for FAT and DIR
format_buffer	ds	2		; for format table
verify_buffer	ds	2		; for reading sector during verify
dec_buff	ds	2		; buffer to convert from Hex to Dec

format		proc
		jsr	init
		clrb
		ldx	#40			; allocate format buffer
		int	allocate
		cpx	#0
		beq	no_memory
		stx	format_buffer
		ldx	#$200			; allocate verify buffer
		int	allocate
		cpx	#0
		beq	no_memory
		stx	verify_buffer		; verify buffer
		ldx	#$600			; allocate buffer for FAT -
		int	allocate		; needed during format
		cpx	#0
		bne	has_memory
no_memory	ldx	#_no_memory
		int	print_string
		int	terminate
has_memory	stx	work_buffer		; FAT buffer

		jsr	set_vectors
start_again	jsr	reset_format
		ldaa	drive
		jsr	print_insert
		jsr	press_enter
		jsr	format_disk
		jsr	make_fat
		bcs	gen_fail
		jsr	make_boot
		bcs	gen_fail
		jsr	make_dir
		bcs	gen_fail
end_format	ldx	#_complete
		int	print_string
		jsr	disk_space
format_another	ldx	#_format_another
		int	print_string
check_again	int	getchar
		anda	#$DF			; uppercase
		cmpa	#'Y'
		bne	check_no
		jmp	start_again
check_no	cmpa	#'N'
		bne	check_again
		bra	bye
gen_fail	ldx	#_general_fail
		int	print_string
bye		jsr	restore_vectors
		int	terminate
;------------------------------------------------
set_vectors
		ldaa	#break
		int	get_vector
		stab	old_break_page
		stx	old_break_off
		ldx	fdd_parms
		stx	old_fdd_parms
		clrb
		ldx	#old_fdd_parms
		ldaa	#9
		int	move
		ldx	#my_fdd_parms
		ldaa	#my_fill_byte
		staa	7,x
		sei
		stx	fdd_parms
		ldaa	#break
		clrb
		ldx	#break_handler
		int	set_vector
		cli
		rts
restore_vectors
		ldx	old_fdd_parms
		stx	fdd_parms
		ldaa	#break
		ldab	old_break_page
		ldx	old_break_off
		int	set_vector
		rts
break_handler
		jsr	restore_vectors
		ldaa	#break
		ldab	old_break_page
		ldx	old_break_off
		int	chain

old_break_page	ds	1
old_break_off	ds	2
old_fdd_parms	ds	2
my_fdd_pointer	dw	my_fdd_parms
my_fdd_parms	ds	9

;-----------------------------------

format_disk
		ldaa	#max_retry
		staa	retry
loopf		ldab	head
		jsr	format_track
		tsta
		bne	error_handler
		ldaa	#max_retry
		staa	retry
loopv		jsr	verify_track
		tsta
		beq	verify_ok
		dec	retry
		bpl	loopv
		tst	track
		bne	mark
		tst	head
		beq	track0_bad     ; if track0 is bad then terminate format
		clr	max_head	; single side disk
		bra	verify_ok
mark		jsr	mark_bad
verify_ok	ldaa	#max_retry
		staa	retry
		inc	head
		ldaa	head
		cmpa	max_head
		bls	loopf
		clr	head
		inc	track
		ldaa	track
		cmpa	max_track
		bls	loopf
		rts
error_handler
		dec	retry
		bpl	loopf
		bita	#%10111100		; bad fdc or seek err
		beq	err1			; or other errors
		ldab	track
		cmpb	#0
		bne	no_track0
track0_bad	ldx	#_track0_bad
		bra	form_err
no_track0	ldx	#_general_fail
		bra	form_err
err1		ldx	#_timeout
		bita	#%01000010		; timeout
		bne	form_err
		ldx	#_write_prot
form_err	int	print_string
		ldx	#_format_fail
		int	print_string
		pula
		pula
		jmp	format_another
;---------------------------------------
format_track
		ldx	format_buffer
		stx	buffer
		ldaa	#1
		staa	count
lp1		ldaa	track
		staa	0,x
		stab	1,x
		ldaa	count
		staa	2,x
		ldaa	#2
		staa	3,x
		inx
		inx
		inx
		inx
		inc	count
		ldaa	sec_per_track
		cmpa	count
		bcc	lp1
		staa	sector
		jsr	print_info
		ldaa	#4
		ldx	#int17_table
		oraa	double_step
		int	floppy_disk
		rts
;-----------------------------------------
verify_track	ldx	verify_buffer
		stx	buffer
		ldab	sec_per_track
		ldaa	#1
		staa	count
lp2		ldx	#interleave_3 - 1
		ldaa	count
		staa	interleave + 1			; self modified !
interleave	ldaa	0,x				; dummy
		staa	sector
		ldaa	#1
		ldx	#int17_table
		oraa	double_step
		int	floppy_disk
		tsta
		bne	end_verify
		inc	count
		cmpb	count
		bcc	lp2
end_verify	rts
;--------------------------------------
print_info	ldx	#_head
		int	print_string
		ldaa	head
		jsr	dec_print
		ldx	#_track
		int	print_string
		ldaa	track
		jsr	dec_print
		rts
;---------------------------------------
dec_print	staa	dec_buff + 1
		ldx	dec_buff
		int	print_dec
		rts
;--------------------------------------
reset_format	clr	track
		clr	head
		ldx	work_buffer
		ldaa	#6		      ; FAT buffer is $600 long
		jsr	clear_buffer
		ldaa	#$80		      ; cold reset fdc
		int	floppy_disk
		rts
;--------------------------------------

interleave_3	db	3,6,9,4,7,1,5,8,2
retry		db	max_retry
count		ds	1
error_code	ds	1
sector_count	ds	1

_no_memory	db	10, 'Insufficient memory',10,00
_format_another db	10, 10, 'Format another (Y/N)?',00
_complete	db	13, 'Format complete       ',0
_head		db	13, 'Head: ',0
_track		db	' Cylinder: ',0
_general_fail	db	13, 'General failure error ',0
_timeout	db	13, 'Drive not ready       ',0
_write_prot	db	13, 'Attempted write-protect violation',00
_track0_bad	db	13, 'Track 0 bad, disk is unusable',0
_format_fail	db	10,10,7, 'Format failure      ',10,0
		endp


press_enter	proc
		ldx	#_strike_enter
		int	print_string
again		int	getchar
		cmpa	#$C0			; enter
		bne	again
		ldaa	#10
		int	print_char
		int	print_char
		int	print_char
		rts
_strike_enter	db	'and strike ENTER when ready',0
		endp

print_insert	proc
		ldx	#_ins_new
		int	print_string
		adda	#$41
		int	print_char
		ldaa	#':'
		int	print_char
		ldaa	#10
		int	print_char
		rts
_ins_new	db	10, 10, 'Insert new diskette for drive ',00
		endp

print_error	proc
		ldaa	#7
		int	print_char
		ldaa	#10
		int	print_char
		int	print_string
		rts
		endp


init		proc
		int	get_dos_version
		cmpb	dos_version
		bne	cmp_dos
		cmpa	dos_version + 1
cmp_dos 	bcc	dos_ok
inv_dos 	ldx	#_inv_dos
		jmp	fatal_err
dos_ok		int	get_paramcount
		deca
		staa	count
		bne	has_drive_lett
		jmp	drive_lett
has_drive_lett	cmpa	#3
		bls	parms_ok
		jmp	inv_parm
parms_ok	ldaa	#1
		staa	param_num
		ldx	#command_line
		int	get_paramstr
		ldab	0,x
		andb	#$DF			; uppercase
		cmpb	#$41
		bcs	drive_lett
		cmpb	#$42
		bhi	drive_lett
		subb	#$41
		stab	drive
		ldab	1,x
		cmpb	#':'
		bne	drive_lett
		ldab	2,x
		beq	end_argv	      ; end of paramstr
continue	inx
		inx
		bra	next_parm
end_argv	inc	param_num
		ldaa	param_num
		dec	count
		beq	end_of_args
		ldx	#command_line
		int	get_paramstr
next_parm	ldab	0,x
		cmpb	#'/'
		bne	inv_parm
		ldab	1,x
		cmpb	#'1'
		beq	one_head
		cmpb	#'4'
		bne	inv_parm
		ldab	2,x
		cmpb	#'0'
		bne	inv_parm
		ldaa	#$80		; this is for double stepping in large
		staa	double_step	; drives. Reserved for future use.
		ldab	3,x
		beq	end_argv
		bra	continue
one_head	clr	max_head
		ldab	2,x
		beq	end_argv
		bra	continue
end_of_args	ldaa	#1
		tst	drive
		beq	drive1
		ldaa	#2
drive1		anda	drive_present
		beq	inv_drive
		anda	drive_type
		beq	small_drive
		tst	double_step
		bne	bye
		ldaa	#79
		staa	max_track
		ldaa	#1
		staa	is_80
		bra	bye
small_drive	tst	double_step
		beq	bye
		clr	double_step
bye		rts
drive_lett	ldx	#_no_drive_lett
		bra	fatal_err
inv_parm	ldx	#_inv_param
		bra	fatal_err
inv_drive	ldx	#_inv_drive
fatal_err	int	print_string
		int	terminate
_inv_param	db	10, 'Invalid parameter',10,00
_inv_drive	db	10, 'Invalid drive specification',10,0
_no_drive_lett	db	10, 'Drive letter must be specified',10,0
_inv_dos	db	10, 'Invalid DOS version',10,0
count		ds	1
param_num	ds	1
command_line	ds	80
		endp

make_boot	proc
		ldab	max_head
		incb
		ldx	#boot_sector
		stab	heads_off,x
		ldaa	#$70
		staa	root_size_off,x
		tst	max_head
		beq	one_head
		tst	is_80
		bne	t80
		ldaa	#$D0
		staa	totalsec_off,x
		ldaa	#$02
		staa	s_per_clust_off,x
		staa	totalsec_off + 1 ,x
		ldaa	#$FD
		staa	mediadesc_off,x
		bra	write_boot
t80		ldaa	#$A0
		staa	totalsec_off,x
		ldaa	#$05
		staa	totalsec_off + 1 ,x
		ldaa	#$02
		staa	s_per_clust_off,x
		ldaa	#$F9
		staa	mediadesc_off,x
		ldaa	#$03
		staa	fat_size_off,x
		bra	write_boot
one_head	ldaa	#$01
		staa	s_per_clust_off,x
		tst	is_80
		bne	tr80
		ldaa	#$68
		staa	totalsec_off,x
		ldaa	#$01
		staa	totalsec_off + 1 ,x
		ldaa	#$40
		staa	root_size_off,x
		ldaa	#$FC
		staa	mediadesc_off,x
		bra	write_boot
tr80		ldaa	#$D0
		staa	totalsec_off,x
		ldaa	#$02
		staa	totalsec_off + 1 ,x
		ldaa	#$F0
		staa	mediadesc_off,x
		ldaa	#$03
		staa	fat_size_off,x
		ldaa	#$70
		staa	root_size_off,x
write_boot	ldaa	#0
		staa	track
		staa	head
		inca
		staa	sector
		ldx	#boot_sector
		stx	buffer
		jsr	write_sector
		rts
boot_sector
		db	  $EB,$4B,$90,$55,$6E,$69,$44,$4F,$53,$20,$20,$00,$02,$02,$01,$00
		db	  $02,$70,$00,$D0,$02,$FD,$02,$00,$09,$00,$02,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$12
		db	  $00,$00,$00,$00,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$E9,$B0,$00
		db	  $BD,$01,$B3,$FF,$01,$AA,$CE,$00,$00,$FF,$01,$AC,$86,$01,$CE,$01
		db	  $AA,$3F,$4A,$4D,$26,$32,$F7,$01,$B2,$17,$CE,$01,$AE,$3F,$51,$FE
		db	  $01,$AE,$26,$24,$FE,$01,$B0,$FF,$01,$AC,$CE,$10,$00,$FF,$01,$AA
		db	  $B6,$01,$B2,$CE,$01,$AA,$3F,$4C,$4D,$26,$0D,$BC,$01,$B0,$26,$08
		db	  $B6,$01,$B2,$3F,$4E,$7E,$10,$00,$CE,$02,$60,$3F,$23,$3F,$20,$7F
		db	  $E6,$C0,$FE,$FF,$FE,$01,$01,$01,$6E,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$CE,$00,$00,$3F,$41,$4D,$26,$07,$C1,$02,$26,$03,$7F
		db	  $EB,$BD,$CE,$02,$AA,$39,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $0E,$1F,$BE,$60,$7D,$FC,$AC,$0A,$C0,$74,$06,$B4,$0E,$CD,$10,$EB
		db	  $F5,$33,$C0,$CD,$16,$CD,$19,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		db	  $0D,$0A,$0A,$4E,$6F,$6E,$2D,$53,$79,$73,$74,$65,$6D,$20,$64,$69
		db	  $73,$6B,$20,$6F,$72,$20,$64,$69,$73,$6B,$20,$65,$72,$72,$6F,$72
		db	  $0D,$0A,$52,$65,$70,$6C,$61,$63,$65,$20,$61,$6E,$64,$20,$73,$74
		db	  $72,$69,$6B,$65,$20,$61,$6E,$79,$20,$6B,$65,$79,$20,$77,$68,$65
		db	  $6E,$20,$72,$65,$61,$64,$79,$0D,$0A,$00,$55,$4E,$49,$44,$4F,$53
		db	  $2E,$43,$4D,$44,$00,$00,$00,$00
		db	  'Format Utility  '
		db	  '  Version 2.0   '
		db	  '(C) Ivan Gorinov'
		db	  ' - Pendgy   1989'
		db	  $00,$00,$00,$00,$00,$00,$A5,$5A
		endp


mark_bad	proc
		ldaa	sec_per_track
		staa	sec
		ldaa	#14	      ; reserved sectors for large disks
		staa	reserved_sec
		tst	is_80
		bne	next_sec
		ldaa	#9	      ; reserved sectors for single sided disks
		staa	reserved_sec
		tst	max_head
		beq	next_sec
		ldaa	#12	      ; reserved sectors for double sided disks
		staa	reserved_sec
next_sec	ldab	track
		tst	max_head
		beq	one_head
		addb	track
one_head	ldaa	head
		aba
		ldab	sec_per_track
		int	mul_A_B
		adda	sec
		adcb	#0
		suba	#1
		sbcb	#0
		suba	reserved_sec
		sbcb	#0
		stab	cluster
		staa	cluster + 1
		ldx	cluster
		ldaa	max_head
		inca
		clrb
		int	div_X_BA
		inx
		stx	cluster
		clr	odd_flag
		ldaa	cluster + 1
		anda	#1
		beq	even
		ldaa	#$80
		staa	odd_flag
even		jsr	mark_cluster
		dec	sec
		bne	next_sec
bye		clc
		rts

mark_cluster	ldab	cluster       ; offset in FAT = (cluster*3)/2
		ldaa	cluster + 1
		adda	cluster + 1
		adcb	cluster
		adda	cluster + 1
		adcb	cluster
		staa	cluster + 1
		stab	cluster
		ldx	cluster
		ldaa	#2
		clrb
		int	div_X_BA
		stx	cluster
		ldx	work_buffer
		stx	fat_buffer
		ldab	cluster
		decb
		bmi	get_cell
		inc	fat_buffer
		decb
		bmi	get_cell
		inc	fat_buffer
		decb
		bmi	get_cell
		inc	fat_buffer
get_cell	ldx	fat_buffer
		ldaa	cluster + 1
		staa	get + 1 		; ! self modified
get		ldx	0,x			; ! dummy
		stx	fat_cell
		tst	odd_flag
		bne	odd
		ldaa	#$F7
		staa	fat_cell
		ldaa	fat_cell + 1
		anda	#$F0
		oraa	#$0F
		staa	fat_cell + 1
		bra	change_fat
odd		ldaa	fat_cell
		anda	#$0F
		oraa	#$70
		staa	fat_cell
		ldaa	#$FF
		staa	fat_cell + 1
change_fat	ldx	fat_buffer
		ldaa	cluster + 1
		staa	change_h + 1		  ; ! self modified
		staa	change_l + 1
		ldab	fat_cell
		ldaa	fat_cell + 1
change_h	stab	0,x			  ; ! dummy
		inx
change_l	staa	0,x			  ; ! dummy
		rts
cluster 	ds	2
fat_cell	ds	2
fat_buffer	ds	2
reserved_sec	ds	1
odd_flag	ds	1
sec		ds	1
		endp

make_fat	proc
		ldaa	#2
		tst	is_80
		bne	t80
		deca
		staa	count
		ldaa	#$FC
		ldab	max_head
		aba
		bra	begin
t80		staa	count
		ldaa	#$F0
		tst	max_head
		beq	begin
		ldaa	#$F9
begin		ldx	work_buffer
		stx	buffer
		staa	0,x
		ldaa	#$FF
		staa	1,x
		staa	2,x
		clr	track
		clr	head
		ldaa	#2
		staa	next_fat_sec
lp1		staa	sector
		jsr	write_sector
		bcs	bye
		ldaa	next_fat_sec
		adda	#2
		tst	is_80
		beq	next1
		inca
next1		staa	sector
		jsr	write_sector
		bcs	bye
		dec	count
		bmi	bye
		inc	buffer
		inc	buffer
		inc	next_fat_sec
		ldaa	next_fat_sec
		bra	lp1
bye		rts
count		ds	1
next_fat_sec	ds	1
		endp

write_sector	proc
		ldab	#max_retry
lp1		ldaa	#2
		ldx	#int17_table
		oraa	double_step
		int	floppy_disk
		tsta
		beq	no_error
		decb
		bpl	lp1
		sec
		rts
no_error	clc
		rts
		endp

clear_buffer	proc
		clr	c1
		staa	c2
		clra
loop		staa	0,x
		inx
		dec	c1
		bne	loop
		dec	c2
		bne	loop
		rts
c1		ds	1
c2		ds	1
		endp

make_dir	proc
		ldaa	#9		 ;sec 9 on track 0 is last dir sector
		staa	end_dir 	 ;for single sided 40 tracks' disks
		ldaa	max_head
		staa	num_of_sides
		ldx	work_buffer
		stx	buffer
		ldaa	#2		      ; DIR buffer is $200 long
		jsr	clear_buffer
		clr	track
		clr	head
		ldaa	#6		; first dir sector on 40 tracks' disks
		tst	is_80
		beq	lp2
		ldaa	#8		; first dir sector on 80 tracks' disks
lp2		staa	sector
lp1		jsr	write_sector
		bcs	bye_bad
		inc	sector
		ldaa	sector
		cmpa	end_dir
		bls	lp1
		ldaa	#1
		staa	sector
		inc	head
		ldaa	#5		; end dir sector on 80 tracks' disks
		staa	end_dir
		tst	is_80
		beq	t40
		tst	num_of_sides
		bmi	bye_ok
		dec	num_of_sides
		bpl	lp1
		clr	head
		inc	track
		bra	lp1
t40		ldaa	#3		; end dir sector on 40 tracks' disks
		dec	num_of_sides
		bmi	bye_ok
		staa	end_dir
		bra	lp1
bye_ok		clc
bye_bad 	rts
end_dir 	ds	1
num_of_sides	ds	1
		endp

disk_space	proc
		ldx	#lint_one
		tst	max_head
		beq	one_head
		ldx	#lint_two
one_head	tst	is_80
		beq	load_lint
		inx			; total space for 80 track disks
		inx			; found in next 4 bytes
		inx
		inx
load_lint	stx	save_x
		ldaa	0,x
		staa	lint+3
		ldaa	1,x
		staa	lint+2
		ldaa	2,x
		staa	lint+1
		ldaa	3,x
		staa	lint
		ldaa	#10
		int	print_char
		int	print_char
		ldaa	#' '
		int	print_char
		int	print_char
		ldx	#lint
		jsr	pr_dec
		ldx	#_total_space
		int	print_string
		ldx	#lint
		ldaa	drive
		int	get_free_space
		ldx	save_x
		ldx	0,x
		cpx	lint
		bne	not_equal
		ldx	save_x
		ldx	2,x
		cpx	lint+2
		beq	no_bad_sec
not_equal	ldx	save_x
		ldaa	3,x
		suba	lint+3
		staa	lint_bad
		ldaa	2,x
		sbca	lint+2
		staa	lint_bad+1
		ldaa	1,x
		sbca	lint+1
		staa	lint_bad+2
		ldaa	0,x
		sbca	lint
		staa	lint_bad+3
		ldx	#lint_bad
		jsr	pr_dec
		ldx	#_bad_space
		int	print_string
no_bad_sec	jsr	swap
		ldx	#lint
		jsr	pr_dec
		ldx	#_avail_space
		int	print_string
		rts
swap		ldaa	lint
		ldab	lint+3
		staa	lint+3
		stab	lint
		ldaa	lint+1
		ldab	lint+2
		staa	lint+2
		stab	lint+1
		rts
_total_space	db	' bytes total disk space',10,'  ',0
_bad_space	db	' bytes in bad sectors',10,'  ',0
_avail_space	db	' bytes available on disk',10,0
lint_one	db	0,2,$BE,0	 ; total space for single sided disks
lint_one80	db	0,5,$84,0	 ; total space for single sided, 80 trk
lint_two	db	0,5,$88,0	 ; total space for double sided disks
lint_two80	db	0,$B,$24,0	 ; total space for double sided, 80 trk
lint_bad	ds	4
lint		ds	4
save_x		ds	2
		endp

pr_dec		proc
		stx	x1
		clr	dec_flag
		ldx	#_10000000
		bsr	subtract
		ldx	#_1000000
		bsr	subtract
		ldx	#_100000
		bsr	subtract
		ldx	#_10000
		bsr	subtract
		ldx	#_1000
		bsr	subtract
		ldx	#_100
		bsr	subtract
		ldx	#_10
		bsr	subtract
		ldaa	#1
		staa	dec_flag
		ldx	#_1
		bra	subtract
subtract
		clrb
		stx	x2
pr_loop 	ldx	x1
		ldaa	x
		ldx	x2
		suba	x
		psha
		ldx	x1
		ldaa	x, 1
		ldx	x2
		sbca	x, 1
		psha
		ldx	x1
		ldaa	x, 2
		ldx	x2
		sbca	x, 2
		psha
		ldx	x1
		ldaa	x, 3
		ldx	x2
		sbca	x, 3
		psha
		bcs	sorry
		ldx	x1
		pula
		staa	x, 3
		pula
		staa	x, 2
		pula
		staa	x, 1
		pula
		staa	x, 0
		incb
		bra	pr_loop
sorry
		ins
		ins
		ins
		ins
		ldaa	#'0'
		aba
		orab	dec_flag
		stab	dec_flag
		bne	pr_it
		ldaa	#' '
pr_it		int	print_char
		rts
_10000000	db	$80,$96,$98,0		 ;$0098 9680 = 10 000 000
_1000000	db	$40,$42,$f,0		 ;$000F 4240 = 1 000 000
_100000 	db	$a0,$86,1,0		 ;$0001 86A0 = 100 000
_10000		db	$10,$27,0,0		 ;$0000 2710 = 10 000
_1000		db	$E8,3,0,0		 ;$0000 03E8 = 1 000
_100		db	$64,0,0,0		 ;$0000 0064 = 100
_10		db	$a,0,0,0		 ;$0000 000A = 10
_1		db	1,0,0,0 		 ;$0000 0001 = 1
dec_flag	ds	1
x1		ds	2
x2		ds	2
		endp
		end
