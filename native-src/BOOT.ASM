code		segment byte public
		assume cs: code, ds: code

		public	boot
		public	parameters

		org	7C00h

boot		proc	near
		jmp	ibm_jump
		db	'UniDOS  '
parameters:
		rept	ibm_jump - $
		db	0
		endm

		org	7C4dh
ibm_jump:	jmp	ibm_start
pyldin:

		db	0CEh, 002h, 0AAh		  ;ldx	   #fname
		db	0FFh,  01h, 0AAh		  ;stx	   table
		db	0CEh,  00h,  00h		  ;ldx	   #0
		db	0FFh,  01h, 0ACh		  ;stx	   table+2
		db	086h,  01h			  ;ldaa    #1
		db	0CEh,  01h, 0AAh		   ;ldx     #table
		db	3Fh, 4Ah			  ;int	   $4A
		db	4Dh				  ;tsta
		db	26h, 32h			  ;bne	   error
		db	0F7h, 01, 0B2h			  ;stab    handle

		db	17h				  ;tba
		db	0CEh, 01, 0AEh			  ;ldx	   #size_buff
		db	3Fh, 51h			  ;int	   $51

		db	0FEh, 01, 0AEh			  ;ldx	   size_buff
		db	26h, 24h			  ;bne	   error
		db	0FEh, 01, 0B0h			  ;ldx	   size_buff+2
		db	0FFh, 01, 0ACh			  ;stx	   table+2
		db	0CEh, 10h, 00			  ;ldx	   #$1000
		db	0FFh, 01, 0AAh			   ;stx     table
		db	0B6h, 01, 0B2h			  ;ldaa    handle
		db	0CEh, 01, 0AAh			   ;ldx     #table
		db	3Fh, 4Ch			  ;int	   $4C
		db	4Dh				  ;tsta
		db	26h, 0Dh			  ;bne	   error
		db	0BCh, 01h, 0B0h 		  ;cpx	   size_buff+2
		db	26h, 08h			 ;bne	  error

		db	0B6h, 01, 0B2h			  ;ldaa    handle
		db	03Fh, 4Eh			  ;int	   $4E
		db	7Eh, 10h, 00			  ;jmp	   $1000
						 ; error
		db	0CEh, 02, 60h			  ;ldx	   #$260
		db	3Fh, 23h			  ;int	   $23
		db	3Fh, 20h			  ;int	   $20

		db	07Fh, 0E6h, 0C0h		  ;clr	   $E6C0
		db	0FEh, 0FFh, 0FEh		  ;ldx	   $FFFE
		db	1, 1, 1 			  ;stx	   $EE00
		db	6Eh, 00 			  ;jmp	   x

		db	4 dup(0)		 ; table  ds	   4
		db	4 dup(0)		 ; size_buff  ds      4
		db	0			 ; handle ds	   1


		rept	ibm_start - $
		db	0
		endm

		org	7d00h
ibm_start:
		push	cs
		pop	ds
		mov	si, 7D60h
		cld
prloop: 	lodsb
		or	al, al
		jz	reboot
		mov	ah, 0Eh
		int	10h
		jmp	prloop
reboot:
		xor	ax, ax
		int	16h
		int	19h
		rept	message - $
		db	0
		endm

		org	7d60h
message 	db	13, 10, 10, 'Non-System disk or disk error'
		db	13, 10
		db	'Replace and strike any key when ready', 13, 10, 0
fname		db	'UNIDOS.CMD', 0

		rept	chksum - $
		db	0
		endm

		org	7DFEh
chksum: 	db	0A5h, 05Ah
boot		endp
code		ends
		end
