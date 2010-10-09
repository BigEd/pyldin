
;   Diskcopy utility for Pyldin 601	     Version 2.0
;
;  (C) Ivan Gorinov   1989

; constants

max_retry	=	2
totalsec_off	=	$13		; offset of total sectors in boot
mediadesc_off	=	$15		; offset of media descriptor in boot
sec_p_trk_off	=	$18		; offset of sectors per track in boot
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
get_vector	=	$2E
set_vector	=	$2F
get_free_mem	=	$36
terminate	=	$38
get_paramcount	=	$3B
get_paramstr	=	$3C
logical_sector	=	$40
get_dos_version =	$41
get_cur_drive	=	$45

; system variables usage

drive_present	=	$ED02
drive_type	=	$ED03

		jmp	diskcopy

dos_version	db	2, 0
		db	'DiskCopy Utility    Version 2.05',0

copyright	db	'(C) çàèã "è∞Æ£∞†¨≠Æ Æ±®£≥∞ø¢†≠•"  1988-89',0
		db	'(C) à¢†≠ ÉÆ∞®≠Æ¢     1989',0

config_byte	db	0		;=0 - 1 drive system, else 2 drives
source_drive	db	0
dest_drive	db	0
double_step_s	db	0		; if=$80 then double stepping needed
double_step_d	db	0		; if=$80 then double stepping needed

diskcopy	proc
		jsr	init
		int	get_free_mem
		tba
		ldab	#8
		suba	#$04
		bcs	no_memory
		anda	#$FE			; page $200, even
		staa	alloc_size
		clr	alloc_size + 1
		ldx	alloc_size
		int	allocate
		cpx	#0
		beq	no_memory
		stx	copy_buffer
		clrb
		ldx	#40
		int	allocate
		cpx	#0
		beq	no_memory
		stx	format_buffer
		clrb
		ldx	#$200
		int	allocate
		cpx	#0
		bne	has_memory
no_memory	ldx	#_no_memory
		int	print_string
		jmp	bye
has_memory	stx	boot_buffer

start_again	jsr	reset_copy
		tst	config_byte
		bpl	next_part
		ldaa	source_drive
		ldx	#_ins_source
		jsr	print_insert
		ldaa	dest_drive
		ldx	#_ins_target
		jsr	print_insert
		jsr	press_key
next_part
		ldaa	source_drive
		staa	drive
		tst	config_byte
		bmi	read_phase
		ldx	#_ins_source
		jsr	print_insert
		jsr	press_key
read_phase	tst	first_flag		; if this is start of copy
		bne	rrr			; then, the boot sector
		jsr	read_boot		; must be read
rrr		jsr	get_start
		ldaa	#1			; read command
		oraa	double_step_s
		staa	rw_command
		jsr	read_write
		ldaa	dest_drive
		staa	drive
		tst	config_byte
		bmi	write_phase
		ldx	#_ins_target
		jsr	print_insert
		jsr	press_key
write_phase	jsr	get_start
		ldaa	#2			; write command
		oraa	double_step_d
		staa	rw_command
		jsr	read_write
		bcs	end_copy
		jsr	next_sector		; test if track < 0
		bcs	end_copy
		ldaa	track
		staa	start_track
		ldaa	head
		staa	start_head
		ldaa	sector_count
		staa	start_sector
		bra	next_part

end_copy	tst	error_flag
		beq	without_err
		ldx	#_may_unusable
		jsr	print_error
without_err	ldx	#_copy_another
		int	print_string
		int	getchar
		anda	#$DF			; uppercase
		cmpa	#'Y'
		bne	check_no
		jmp	start_again
check_no	cmpa	#'N'
		bne	without_err
bye		int	terminate

;-----------------------------------
read_write	ldx	copy_buffer
		stx	buffer
		ldaa	alloc_size
		lsra					; divide by 2
		staa	buffer_counter
loop_rw 	ldx	#interleave_3 - 1
		ldaa	sector_count
		staa	interleave + 1			; self modified !
interleave	ldaa	0,x				; dummy
		staa	sector
		ldaa	rw_command
		ldx	#int17_table
		int	floppy_disk
		tsta
		bne	error_handler
no_error	ldaa	#max_retry
		staa	retry
		inc	buffer
		inc	buffer
		dec	buffer_counter
		beq	end_rw
		jsr	next_sector
		bcc	loop_rw
		rts
end_rw		clc
		rts
error_handler
		staa	error_code
		dec	retry
		bpl	loop_rw
		bita	#%10100000		; bad fdc or seek err
		beq	err1
		ldx	#_general_fail
		jsr	print_error
		int	terminate		; fatal error
err1		ldaa	error_code
		bita	#%01000010		; timeout
		beq	err2
		ldaa	rw_command
		anda	#$7F			; mask double step bit
		cmpa	#1			; read, no format when second
		beq	tm_out			; timeout has been occured
		tst	timeout_flag
		bmi	fmt_need
		ldaa	#$80
		staa	timeout_flag
tm_out		ldx	#_timeout
continue	jsr	print_error
		jsr	press_key
		jmp	loop_rw
err2		ldaa	error_code		; write protected
		ldx	#_write_prot
		bita	#%00000001
		bne	continue
		ldab	rw_command
		andb	#$7F			; mask double step bit
		ldx	#_read_error
		cmpb	#1
		beq	err4
		tst	format_flag
		bne	err3
fmt_need	ldaa	#$80
		staa	format_flag
		ldaa	#0
		staa	timeout_flag
		ldx	#_formatting
		int	print_string
		ldaa	sec_per_track
		psha
		ldaa	max_head
		psha
		ldaa	max_track
		psha
		ldaa	track
		psha
		ldx	format_buffer
		ldab	head
		ldaa	drive
		jsr	format
		jmp	loop_rw
err3		ldx	#_write_error
err4		jsr	print_error
		ldx	#_track
		ldaa	track
		int	print_string
		jsr	dec_print
		ldx	#_head
		ldaa	head
		int	print_string
		jsr	dec_print
		ldaa	#$80
		staa	error_flag
		jmp	no_error
;--------------------------------------
next_sector	ldaa	sec_per_track
		inc	sector_count
		cmpa	sector_count
		bcc	ret
		ldaa	#1
		staa	sector_count
		inc	head
		ldaa	max_head
		cmpa	head
		bcc	ret
		clra
		staa	head
		inc	track
		ldaa	max_track
		cmpa	track			; if carry clear - continue
ret		rts				; if set end of copy
;--------------------------------------
get_start	ldaa	start_track
		staa	track
		ldaa	start_head
		staa	head
		ldaa	start_sector
		staa	sector_count
		rts
;--------------------------------------
read_boot	clra
		staa	track
		staa	head
		ldaa	source_drive
		staa	drive
		ldaa	#1
		staa	sector
		ldx	boot_buffer
		stx	buffer
		ldx	#int17_table
		int	floppy_disk
		tsta
		bne	boot_err
		ldx	boot_buffer
		ldaa	totalsec_off,x
		ldab	totalsec_off + 1,x
		staa	total_sec + 1
		stab	total_sec
		ldaa	heads_off,x
		deca
		bmi	boot_err
		staa	max_head
		inca
		ldab	sec_p_trk_off,x
		beq	boot_err
		stab	sec_per_track
		int	mul_A_B
		ldx	total_sec
		int	div_X_BA
		tsta
		bne	boot_err
		tstb
		bne	boot_err
		stx	total_sec
		ldaa	total_sec + 1
		deca
		staa	max_track
		cmpa	#39
		bls	t40
		ldaa	source_drive
		inca
		anda	drive_type
		bne	good_s_drive
		ldx	#_bad_drive
		bra	bye_err
good_s_drive	ldaa	dest_drive
		inca
		anda	drive_type
		bne	print_copy
		ldx	#_incomp_drives
		bra	bye_err
t40		ldaa	source_drive
		inca
		anda	drive_type
		beq	small_drive
		ldaa	#$80
small_drive	staa	double_step_s
		ldaa	dest_drive
		inca
		anda	drive_type
		beq	small_drv
		ldaa	#$80
small_drv	staa	double_step_d
		bra	print_copy
boot_err	ldx	#_bad_source
bye_err 	int	print_string
		ldx	#_end_copy
		int	print_string
		pula				; pop last return address and
		pula				; jump to end of current copy
		jmp	end_copy
print_copy	ldx	#_copying
		int	print_string
		ldaa	max_track
		inca
		jsr	dec_print
		ldx	#_tracks
		int	print_string
		ldaa	sec_per_track
		jsr	dec_print
		ldx	#_sec_per_trk
		int	print_string
		ldaa	max_head
		inca
		jsr	dec_print
		ldx	#_sides
		int	print_string
		ldaa	#$80
		staa	first_flag
		rts
dec_print	staa	dec_buff + 1
		ldx	dec_buff
		int	print_dec
		rts
total_sec	ds	2
_copying	db	10, 10, 10, 'Copying ',00
_tracks 	db	' tracks',10,10,00
_sec_per_trk	db	' Sectors/Track, ',00
_sides		db	' Side(s)',00
_bad_source	db	10,10,7,'SOURCE diskette bad or incompatible',10,10,0
_bad_drive	db	10,10,7,'SOURCE diskette and drive incompatible',10,10,0
_incomp_drives	db	10,10,7,'SOURCE diskette and TARGET drive ',10
		db	'incompatible',10,10,0
_end_copy	db	'Copy process ended',0
;--------------------------------------
reset_copy	clr	start_track
		clr	start_head
		clr	error_flag
		clr	format_flag
		clr	timeout_flag
		clr	first_flag
		ldaa	#1
		staa	max_head		; set default values
		staa	start_sector
		ldaa	#39
		staa	max_track
		ldaa	#9
		staa	sec_per_track
		ldaa	#0			; reset fdc
		int	floppy_disk
		rts
;--------------------------------------
error_flag	db	0		; these flags must be initialised
format_flag	db	0		; before start of next copy
timeout_flag	db	0
first_flag	db	0
start_track	db	0
start_head	db	0
start_sector	db	0


interleave_3	db	3,6,9,4,7,1,5,8,2
sec_per_track	db	9
max_head	db	1
max_track	db	39
retry		db	max_retry
rw_command	ds	1
error_code	ds	1
buffer_counter	ds	1
copy_buffer	ds	2
format_buffer	ds	2
boot_buffer	ds	2
alloc_size	ds	2
sector_count	ds	1
dec_buff	ds	2
int17_table
drive		ds	1
track		ds	1
head		ds	1
sector		ds	1
buffer		ds	2

_inv_drive	db	10, 'Invalid drive specification',00
_no_memory	db	10, 'Insufficient memory',10,00
_ins_source	db	10, 10, 'Insert SOURCE diskette in drive ',00
_ins_target	db	10, 10, 'Insert TARGET diskette in drive ',00
_copy_another	db	10, 10, 10, 'Copy another diskette (Y/N)?',00
_formatting	db	10, 10, 'Formatting while copying . . .',00
_track		db	10, 10, 'Track: ',0
_head		db	'      Head: ',0
_may_unusable	db	10, 10, 'Target diskette may be unusable',00
_general_fail	db	10, 'General failure error',10,00
_timeout	db	10, 10, 'Make sure a diskette is inserted into',10
		db	10, 'the drive and the door is closed',00
_read_error	db	10, 10, 'Unrecoverable read error',00
_write_prot	db	10, 10, 'Target diskette is write protected',00
_write_error	db	10, 10, 'Error writing track ',00
		endp


press_key	proc
		ldx	#_press_key
		int	print_string
		int	getchar
		rts
_press_key	db	10, 10, 'Press any key when ready . . .',00
		endp

print_insert	proc
		int	print_string
		adda	#$41
		int	print_char
		ldaa	#':'
		int	print_char
		ldaa	#10
		int	print_char
		rts
		endp

print_error	proc
		ldaa	#7
		int	print_char
		ldaa	#10
		int	print_char
		int	print_string
		rts
		endp

format		proc			;b - head, a - drive, stack - track
		stx	buffer
		stab	head		;sec_per_track, max_head, max_track
		staa	drive		;are also in stack
		pula			    ; pop return address from the stack
		staa	save_ret_addr + 1
		pula
		staa	save_ret_addr
		pula			; pop required common data from stack
		staa	track
		pula
		staa	max_track
		pula
		staa	max_head
		pula
		staa	sec_per_track
		ldaa	save_ret_addr		; push return address
		psha				; to stack
		ldaa	save_ret_addr + 1
		psha
		clra
		int	floppy_disk	; reset fdc
		ldaa	#3
		staa	retry
loop		ldab	head
		jsr	format_track
		tsta
		bne	form_err
		ldaa	#3
		staa	retry
		inc	head
		ldaa	head
		cmpa	max_head
		bls	loop
		clr	head
		inc	track
		ldaa	track
		cmpa	max_track
		bls	loop
		rts
form_err	dec	retry
		bpl	loop
		ldx	#_format_error
		int	print_string
		int	terminate		  ; fatal error, terminate
format_track	ldx	buffer
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
		ldaa	#4
		oraa	double_step_d
		ldx	#int17_table
		int	floppy_disk
		rts
save_ret_addr	ds	2
sec_per_track	ds	1
max_head	ds	1
max_track	ds	1
count		ds	1
retry		ds	1
int17_table
drive		ds	1
track		ds	1
head		ds	1
sector		ds	1
buffer		ds	2
_format_error	db	10,10,10,7, 'Format failure, disk is unusable.',10,0
		endp

init		proc
		int	get_dos_version
		cmpb	dos_version
		bne	cmp_dos
		cmpa	dos_version + 1
cmp_dos 	bcc	dos_ok
inv_dos 	ldx	#_inv_dos
		jmp	fatal_err
dos_ok		int	get_cur_drive
		staa	source_drive
		staa	dest_drive
		clra
		staa	config_byte		; one drive system
		int	get_paramcount
		deca
		beq	bye			; no command line parameters
		cmpa	#2
		bhi	inv_param
		jsr	get_param
		bcs	inv_param
		cmpa	#1
		bne	two_parm
		stab	source_drive
		bra	no_more
two_parm	stab	dest_drive
		deca
		jsr	get_param
		bcs	inv_param
		stab	source_drive
no_more 	cmpb	dest_drive
		beq	bye
		ldaa	#$80			; two drives system
		staa	config_byte
bye		ldaa	drive_present
		anda	#2
		bne	drive2_present
		staa	config_byte
		staa	source_drive
		staa	dest_drive
drive2_present	rts
inv_param	ldx	#_inv_param
fatal_err	int	print_string
		int	terminate
_inv_param	db	10, 'Invalid parameters',10,0
_inv_dos	db	10, 'Invalid DOS version',10,0
		endp

get_param	proc
		ldx	#command_line
		int	get_paramstr
		ldab	command_line
		andb	#$DF			; uppercase
		cmpb	#$41
		bcs	err_param
		cmpb	#$42
		bhi	err_param
		subb	#$41
		clc
		rts
err_param	sec
		rts
command_line	ds	80
		endp
		end
