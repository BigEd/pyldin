data		segment
		extrn	drive: byte
data		ends

code		segment
		assume	cs:code, ds:data

		public	abs_read
		public	abs_write

abs_read	proc	near
		push	bp
		mov	bp, sp
		push	ds

		mov	al, drive
		mov	cx, 1
		mov	dx, 0
		mov	bx, [bp + 4]
		mov	ds, [bp + 6]
		int	25h
		pop	bx
		jc	error1
		xor	ax, ax
error1:
		pop	ds
		pop	bp
		ret
abs_read	endp

abs_write	proc	near
		push	bp
		mov	bp, sp
		push	ds

		mov	al, drive
		mov	cx, 1
		mov	dx, 0
		mov	bx, [bp + 4]
		mov	ds, [bp + 6]
		int	26h
		pop	bx
		jc	error2
		xor	ax, ax
error2:
		pop	ds
		pop	bp
		ret
abs_write	endp

code		ends
		end
