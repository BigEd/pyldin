/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * MC6800 core version 2
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "core/mc6800.h"
#include "core/opcode.h"
#include "core/swiemu.h"
#include "core/devices.h"

#ifdef __GNUC__
#define INLINE inline
#endif

static	byte	*MEM;

static	word	fWai;			// установлен после WAI

	//registers here
static	word	EAR;
static	word	PC;
static	word	SP;
static	word	X;
static	byte	A;
static	byte	B;
//static	byte	P;
	//flags register here
static	byte	c;
static	byte	v;
static	byte	z;
static	byte	n;
static	byte	i;
static	byte	h;

static	dword	mc6800_global_takts;

static int IRQrequest = 0;

static unsigned char mpu_cycles[] = {
/*     00  01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f */
/*00*/ 02,  2, 02, 02, 02, 02,  2,  2,  4,  4,  2,  2,  2,  2,  2,  2,
/*01*/  2,  2, 02, 02, 02, 02,  2,  2, 02,  2, 02,  2, 02, 02, 02, 02,
/*02*/  4, 02,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
/*03*/  4,  4,  4,  4,  4,  4,  4,  4, 02,  5, 02, 10, 02, 02,  9, 12,
/*04*/  2, 02, 02,  2,  2, 02,  2,  2,  2,  2,  2, 02,  2,  2, 02,  2,
/*05*/  2, 02, 02,  2,  2, 02,  2,  2,  2,  2,  2, 02,  2,  2, 02,  2,
/*06*/  7, 02, 02,  7,  7, 02,  7,  7,  7,  7,  7, 02,  7,  7,  4,  7,
/*07*/  6, 02, 02,  6,  6, 02,  6,  6,  6,  6,  6, 02,  6,  6,  3,  6,
/*08*/  2,  2,  2, 02,  2,  2,  2, 02,  2,  2,  2,  2,  3,  8,  3, 02,
/*09*/  3,  3,  3, 02,  3,  3,  3,  4,  3,  3,  3,  3,  4, 02,  4,  5,
/*0a*/  5,  5,  5, 02,  5,  5,  5,  6,  5,  5,  5,  5,  6,  8,  6,  7,
/*0b*/  4,  4,  4, 02,  4,  4,  4,  5,  4,  4,  4,  4,  5,  9,  5,  6,
/*0c*/  2,  2,  2, 02,  2,  2,  2, 02,  2,  2,  2,  2, 02, 02,  3, 02,
/*0d*/  3,  3,  3, 02,  3,  3,  3,  4,  3,  3,  3,  3, 02, 02,  4,  5,
/*0e*/  5,  5,  5, 02,  5,  5,  5,  6,  5,  5,  5,  5, 02, 02,  6,  7,
/*0f*/  4,  4,  4, 02,  4,  4,  4,  5,  4,  4,  4,  4, 02, 02,  5,  6
};

int mc6800_init(void)
{
    mc6800_global_takts = 0;

    MEM = (byte *) get_cpu_mem(65536);
    memset(MEM, 0, 65536);

    devices_init();

    return 0;
}

int mc6800_fini(void)
{
    devices_fini();

    return 0;
}

void mc6800_reset()
{
    devices_reset();
    i = fWai = IRQrequest = 0;
    PC = mc6800_memr(0xfffe)<<8; 
    PC |= mc6800_memr(0xffff);
}

O_INLINE void mc6800_setIrq(int l)
{
    IRQrequest = l;
}

O_INLINE dword mc6800_get_takts(void)
{
    return mc6800_global_takts;
}

O_INLINE byte *mc6800_get_memory(void)
{
    return MEM;
}

O_INLINE byte mc6800_memr(word a)
{
    byte t = 0xff;

    if (devices_memr(a, &t))
	return t;

    return MEM[a];
}

O_INLINE void mc6800_memw(word a, byte d)
{
    if (devices_memw(a, d))
	return;

    MEM[a] = d;	//иначе запись в основную память
}

static INLINE void TestByte(byte b)
{
    if (b == 0) 
	z = 1; 
    else 
	z = 0;
	
    if (b & 0x80) 
	n = 1; 
    else 
	n = 0;
}

static INLINE void TestWord(word w)
{
    if (w == 0) 
	z = 1; 
    else 
	z = 0;

    if (w & 0x8000) 
	n = 1; 
    else 
	n = 0;
}

static INLINE void Bcpx(word a, word b)
{
    int ans = (((a >> 8) - (b >> 8)) << 8) | ((a - b) & 255);
    //int wans = a - b;
    //int ans = wans & 0xffff;

    TestWord(ans);

    if (((a ^ b) & (a ^ ans) & 0x8000) != 0 )
	v = 1; 
    else 
	v = 0;
}

static INLINE byte Blsr(byte a)
{
    byte r = a>>1;

    v = c = a & 1;

    TestByte(r);

    return r;
}

static INLINE byte Basr(byte a)
{
    byte r = (a & 0x80) | (a>>1);

    c = a & 1;

    if (c == (r & 0x80)>>7) 
	v = 0; 
    else 
	v = 1;

    TestByte(r);

    return r;
}

static INLINE byte Basl(byte a)
{
    byte r = a<<1;

    c = (a & 0x80)>>7;

    if (c == (a & 0x40)>>6) 
	v = 0; 
    else 
	v = 1;

    TestByte(r);

    return r;
}

static INLINE byte Bror(byte a)
{
    byte r = (a>>1) | (c<<7);

    c = a & 1;

    if (c == (r & 0x80)>>7) 
	v = 0; 
    else 
	v = 1;

    TestByte(r);

    return r;
}

static INLINE byte Brol(byte a)
{
    byte r = (a<<1) | c;

    c = (a & 0x80)>>7;

    if (c == (a & 0x40)>>6) 
	v = 0; 
    else 
	v = 1;

    TestByte(r);

    return r;
}

static INLINE byte Bsub(byte o1, byte o2)
{
    int op1 = o1;
    int op2 = o2;

    int	wans = op1 - op2;
    int	ans = wans & 0xff;

    TestByte(ans);

    if ((wans & 0x100) != 0) 
	c = 1; 
    else 
	c = 0;

    if (((op1 ^ op2) & (op1 ^ ans) & 0x80) != 0 ) 
	v = 1; 
    else 
	v = 0;

    return (byte) ans;
}

static INLINE byte Bsubc(byte o1, byte o2)
{
    int op1 = o1;
    int op2 = o2;

    int	wans = op1 - op2 - (c ? 1 : 0);
    int	ans = wans & 0xff;

    TestByte(ans);

    if ((wans & 0x100) != 0) 
	c = 1; 
    else 
	c = 0;

    if (((op1 ^ op2) & (op1 ^ ans) & 0x80) != 0) 
	v = 1; 
    else 
	v = 0;

    return (byte) ans;
}

static INLINE byte Badd(byte o1, byte o2)
{
    int op1 = o1;
    int op2 = o2;

    int	wans = op1 + op2;
    int	ans = wans & 0xff;

    TestByte(ans);

    if ((wans & 0x100) != 0) 
	c = 1; 
    else 
	c = 0;
    
    if (((op1 ^ ~op2) & (op1 ^ ans) & 0x80) != 0) 
	v = 1; 
    else 
	v = 0;
    
    if ((((op1 & 0x0f) + (op2 & 0x0f)) & 0x10) != 0) 
	h = 1; 
    else 
	h = 0;
    
    return (byte) ans;
}

static INLINE byte Baddc(byte o1, byte o2)
{
    int op1 = o1;
    int op2 = o2;

    int	wans = op1 + op2 + (c ? 1 : 0);
    int	ans = wans & 0xff;

    TestByte(ans);

    if ((wans & 0x100) != 0) 
	c = 1; 
    else 
	c = 0;
    
    if (((op1 ^ ~op2) & (op1 ^ ans) & 0x80) != 0) 
	v = 1; 
    else 
	v = 0;
    
    if ((((op1 & 0x0f) + (op2 & 0x0f) + (c ? 1 : 0)) & 0x10) != 0) 
	h = 1; 
    else 
	h = 0;
    
    return (byte) ans;
}

static INLINE void Bdaa()
{
    byte	incr = 0;
    byte	carry = c;

    if (h || ((A & 0x0f) > 0x09)) 
	incr |= 0x06;
    
    if (carry || (A > 0x99)) 
	incr |= 0x60;
    
    if (A > 0x99) 
	carry = 1;
    
    A = Badd(A, incr);
    
    c = carry;
}

static INLINE byte NextByte()
{
    return mc6800_memr(PC++);
}

static INLINE void FetchAddr()
{
    EAR = NextByte() << 8;
    EAR = EAR | NextByte();
}

static INLINE void Branch()
{
    if ((EAR & 0x80) == 0) 
	EAR &= 0xFF; 
    else 
	EAR |= 0xFF00;

    PC += EAR;
}

int mc6800_step()
{
    byte oc, oh, t;
    word ofs, r16;

    int	takt = 0;

    if (fWai == 1 && IRQrequest == 0) {
	mc6800_global_takts += 4;
	return 4;
    } else if (fWai == 1 && IRQrequest != 0) {
	fWai = 0;
	mc6800_global_takts += 4;
	return 4;
    } else if (IRQrequest != 0 && i == 0) {
	t = (c?1:0)|(v?2:0)|(z?4:0)|(n?8:0)|(i?16:0)|(h?32:0)|0xc0;
	mc6800_memw(SP--, PC&0xff); mc6800_memw(SP--, PC>>8);
	mc6800_memw(SP--, X&0xff); mc6800_memw(SP--, X>>8);
	mc6800_memw(SP--, A);
	mc6800_memw(SP--, B);
	mc6800_memw(SP--, t);
	PC = mc6800_memr(0xfff8)<<8; PC |= mc6800_memr(0xfff9);
	IRQrequest = 0;
	mc6800_global_takts += 12;
	return 12;
    }

    byte opnum = mc6800_memr(PC++);

    takt = mpu_cycles[opnum];

    switch (opnum) {
	case CLC:	c = 0; break;
	case CLI:	i = 0; break;
	case CLV:	v = 0; break;
	case SEC:	c = 1; break;
	case SEI:	i = 1; break;
	case SEV:	v = 1; break;
	case TPA:	A = (c?1:0)|(v?2:0)|(z?4:0)|(n?8:0)|(i?16:0)|(h?32:0)|0xc0; break;
	case TAP:	c=(A&1)!=0; v=(A&2)!=0; z=(A&4)!=0; n=(A&8)!=0; i=(A&16)!=0; h=(A&32)!=0; break;
	case TBA:	A = B; v = 0; TestByte(A); break;
	case TAB:	B = A; v = 0; TestByte(A); break;
	case TSX:	X = SP + 1; break;
	case TXS:	SP = X - 1; break;

	case DAA:	oh=h; Bdaa(); h=oh; break;

	case PSHA:	mc6800_memw(SP--, A); break;
	case PSHB:	mc6800_memw(SP--, B); break;
	case PULA:	A = mc6800_memr(++SP); break;
	case PULB:	B = mc6800_memr(++SP); break;

	case DEC_idx:	ofs=NextByte()+X; oc=c; mc6800_memw(ofs,Bsub(mc6800_memr(ofs),1)); c=oc; break;
	case DEC:	FetchAddr(); oc=c; mc6800_memw(EAR, Bsub(mc6800_memr(EAR),1)); c=oc; break;
	case DECA:	oc=c; A=Bsub(A,1); c=oc; break;
	case DECB:	oc=c; B=Bsub(B,1); c=oc; break;
	case DES:	SP--; break;
	case DEX:	X--; z = X?0:1; break;

	case INC_idx:	ofs=NextByte()+X; oh=h; oc=c; mc6800_memw(ofs,Badd(mc6800_memr(ofs),1)); c=oc; h=oh; break;
	case INC:	FetchAddr(); oh=h; oc=c; mc6800_memw(EAR, Badd(mc6800_memr(EAR),1)); c=oc; h=oh; break;
	case INCA:	oh=h; oc=c; A=Badd(A,1); c=oc; h=oh; break;
	case INCB:	oh=h; oc=c; B=Badd(B,1); c=oc; h=oh; break;
	case INS:	SP++; break;
	case INX:	X++; z = X?0:1; break;

	case CLR_idx:	mc6800_memw(NextByte()+X, 0); n = v = c = 0; z = 1; break;
	case CLR:	FetchAddr(); mc6800_memw(EAR, 0); n = v = c = 0; z = 1; break;
	case CLRA:	A = n = v = c = 0; z = 1; break;
	case CLRB:	B = n = v = c = 0; z = 1; break;

	case COM_idx: ofs = NextByte()+X; mc6800_memw(ofs, ~mc6800_memr(ofs)); c = 1; v = 0; TestByte(mc6800_memr(ofs)); break;
	case COM:	FetchAddr(); mc6800_memw(EAR, ~mc6800_memr(EAR)); c = 1; v = 0; TestByte(mc6800_memr(EAR)); break;
	case COMA:	A = ~A; c = 1; v = 0; TestByte(A); break;
	case COMB:	B = ~B; c = 1; v = 0; TestByte(B); break;

	case NEG_idx: ofs = NextByte()+X; mc6800_memw(ofs, Bsub(0, mc6800_memr(ofs))); break;
	case NEG:	FetchAddr(); mc6800_memw(EAR, Bsub(0, mc6800_memr(EAR))); break;
	case NEGA:	A = Bsub(0, A); break;
	case NEGB:	B = Bsub(0, B); break;

	case LDAA_imm: A = NextByte(); v = 0; TestByte(A); break;
	case LDAA_dir: A = mc6800_memr(NextByte()); v = 0; TestByte(A); break;
	case LDAA_idx: A = mc6800_memr(X + NextByte()); v = 0; TestByte(A); break;
	case LDAA:	FetchAddr(); A = mc6800_memr(EAR); v = 0; TestByte(A); break;
	case LDAB_imm: B = NextByte(); v = 0; TestByte(B); break;
	case LDAB_dir: B = mc6800_memr(NextByte()); v = 0; TestByte(B); break;
	case LDAB_idx: B = mc6800_memr(X + NextByte()); v = 0; TestByte(B); break;
	case LDAB:	FetchAddr(); B = mc6800_memr(EAR); v = 0; TestByte(B); break;

	case LDS_imm: FetchAddr(); SP = EAR; v = 0; TestWord(SP); break;
	case LDS_dir: ofs=NextByte(); SP = mc6800_memr(ofs) << 8; SP |= mc6800_memr(ofs + 1); v = 0; TestWord(SP); break;
	case LDS_idx: ofs=X+NextByte(); SP = mc6800_memr(ofs) << 8; SP |= mc6800_memr(ofs + 1); v = 0; TestWord(SP); break;
	case LDS:	FetchAddr(); SP = mc6800_memr(EAR) << 8; SP |= mc6800_memr(EAR + 1); v = 0; TestWord(SP); break;
	case LDX_imm: FetchAddr(); X = EAR; v = 0; TestWord(X); break;
	case LDX_dir: ofs=NextByte(); X = mc6800_memr(ofs) << 8; X |= mc6800_memr(ofs + 1); v = 0; TestWord(X); break;
	case LDX_idx: ofs=X+NextByte(); X=mc6800_memr(ofs)<<8; X|=mc6800_memr(ofs+1); v = 0; TestWord(X); break;
	case LDX:	FetchAddr(); X = mc6800_memr(EAR) << 8; X |= mc6800_memr(EAR + 1); v = 0; TestWord(X); break;

	case STAA_dir: mc6800_memw(NextByte(), A); v = 0; TestByte(A); break;
	case STAA_idx: mc6800_memw(X + NextByte(), A); v = 0; TestByte(A); break;
	case STAA:	FetchAddr(); mc6800_memw(EAR, A); v = 0; TestByte(A); break;
	case STAB_dir: mc6800_memw(NextByte(), B); v = 0; TestByte(B); break;
	case STAB_idx: mc6800_memw(X + NextByte(), B); v = 0; TestByte(B); break;
	case STAB:	FetchAddr(); mc6800_memw(EAR, B); v = 0; TestByte(B); break;

	case STS_dir: ofs=NextByte(); mc6800_memw(ofs,SP>>8); mc6800_memw(ofs+1, SP&0xff); v=0; TestWord(SP); break;
	case STS_idx: ofs=X+NextByte(); mc6800_memw(ofs,SP>>8); mc6800_memw(ofs+1,SP&0xff); v=0; TestWord(SP); break;
	case STS:	FetchAddr(); mc6800_memw(EAR,SP>>8); mc6800_memw(EAR+1,SP&0xff); v=0; TestWord(SP); break;
	case STX_dir: ofs=NextByte(); mc6800_memw(ofs,X>>8); mc6800_memw(ofs+1, X&0xff); v=0; TestWord(X); break;
	case STX_idx: ofs=X+NextByte(); mc6800_memw(ofs,X>>8); mc6800_memw(ofs+1,X&0xff); v=0; TestWord(X); break;
	case STX:	FetchAddr(); mc6800_memw(EAR,X>>8); mc6800_memw(EAR+1,X&0xff); v=0; TestWord(X); break;

	case ABA:	A = Badd(A, B); break;

	case ADCA_imm: A = Baddc(A, NextByte()); break;
	case ADCA_dir: A = Baddc(A, mc6800_memr(NextByte())); break;
	case ADCA_idx: A = Baddc(A, mc6800_memr(X + NextByte())); break;
	case ADCA:	FetchAddr(); A = Baddc(A, mc6800_memr(EAR)); break;
	case ADCB_imm: B = Baddc(B, NextByte()); break;
	case ADCB_dir: B = Baddc(B, mc6800_memr(NextByte())); break;
	case ADCB_idx: B = Baddc(B, mc6800_memr(X + NextByte())); break;
	case ADCB:	FetchAddr(); B = Baddc(B, mc6800_memr(EAR)); break;

	case ADDA_imm: A = Badd(A, NextByte()); break;
	case ADDA_dir: A = Badd(A, mc6800_memr(NextByte())); break;
	case ADDA_idx: A = Badd(A, mc6800_memr(X + NextByte())); break;
	case ADDA:	FetchAddr(); A = Badd(A, mc6800_memr(EAR)); break;
	case ADDB_imm: B = Badd(B, NextByte()); break;
	case ADDB_dir: B = Badd(B, mc6800_memr(NextByte())); break;
	case ADDB_idx: B = Badd(B, mc6800_memr(X + NextByte())); break;
	case ADDB:	FetchAddr(); B = Badd(B, mc6800_memr(EAR)); break;

	case SBA:	A = Bsub(A, B); break;

	case SBCA_imm: A = Bsubc(A, NextByte()); break;
	case SBCA_dir: A = Bsubc(A, mc6800_memr(NextByte())); break;
	case SBCA_idx: A = Bsubc(A, mc6800_memr(X + NextByte())); break;
	case SBCA:	FetchAddr(); A = Bsubc(A, mc6800_memr(EAR)); break;
	case SBCB_imm: B = Bsubc(B, NextByte()); break;
	case SBCB_dir: B = Bsubc(B, mc6800_memr(NextByte())); break;
	case SBCB_idx: B = Bsubc(B, mc6800_memr(X + NextByte())); break;
	case SBCB:	FetchAddr(); B = Bsubc(B, mc6800_memr(EAR)); break;

	case SUBA_imm: A = Bsub(A, NextByte()); break;
	case SUBA_dir: A = Bsub(A, mc6800_memr(NextByte())); break;
	case SUBA_idx: A = Bsub(A, mc6800_memr(X + NextByte())); break;
	case SUBA:	FetchAddr(); A = Bsub(A, mc6800_memr(EAR)); break;
	case SUBB_imm: B = Bsub(B, NextByte()); break;
	case SUBB_dir: B = Bsub(B, mc6800_memr(NextByte())); break;
	case SUBB_idx: B = Bsub(B, mc6800_memr(X + NextByte())); break;
	case SUBB:	FetchAddr(); B = Bsub(B, mc6800_memr(EAR)); break;

	case ANDA_imm: A &= NextByte(); v = 0; TestByte(A); break;
	case ANDA_dir: A &= mc6800_memr(NextByte()); v = 0; TestByte(A); break;
	case ANDA_idx: A &= mc6800_memr(X + NextByte()); v = 0; TestByte(A); break;
	case ANDA:	FetchAddr(); A &= mc6800_memr(EAR); v = 0; TestByte(A); break;
	case ANDB_imm: B &= NextByte(); v = 0; TestByte(B); break;
	case ANDB_dir: B &= mc6800_memr(NextByte()); v = 0; TestByte(B); break;
	case ANDB_idx: B &= mc6800_memr(X + NextByte()); v = 0; TestByte(B); break;
	case ANDB:	FetchAddr(); B &= mc6800_memr(EAR); v = 0; TestByte(B); break;

	case ORAA_imm: A |= NextByte(); v = 0; TestByte(A); break;
	case ORAA_dir: A |= mc6800_memr(NextByte()); v = 0; TestByte(A); break;
	case ORAA_idx: A |= mc6800_memr(X + NextByte()); v = 0; TestByte(A); break;
	case ORAA:	FetchAddr(); A |= mc6800_memr(EAR); v = 0; TestByte(A); break;
	case ORAB_imm: B |= NextByte(); v = 0; TestByte(B); break;
	case ORAB_dir: B |= mc6800_memr(NextByte()); v = 0; TestByte(B); break;
	case ORAB_idx: B |= mc6800_memr(X + NextByte()); v = 0; TestByte(B); break;
	case ORAB:	FetchAddr(); B |= mc6800_memr(EAR); v = 0; TestByte(B); break;

	case EORA_imm: A ^= NextByte(); v = 0; TestByte(A); break;
	case EORA_dir: A ^= mc6800_memr(NextByte()); v = 0; TestByte(A); break;
	case EORA_idx: A ^= mc6800_memr(X + NextByte()); v = 0; TestByte(A); break;
	case EORA:	FetchAddr(); A ^= mc6800_memr(EAR); v = 0; TestByte(A); break;
	case EORB_imm: B ^= NextByte(); v = 0; TestByte(B); break;
	case EORB_dir: B ^= mc6800_memr(NextByte()); v = 0; TestByte(B); break;
	case EORB_idx: B ^= mc6800_memr(X + NextByte()); v = 0; TestByte(B); break;
	case EORB:	FetchAddr(); B ^= mc6800_memr(EAR); v = 0; TestByte(B); break;

	case LSR_idx: ofs=X+NextByte(); mc6800_memw(ofs, Blsr(mc6800_memr(ofs))); break;
	case LSR: FetchAddr(); mc6800_memw(EAR, Blsr(mc6800_memr(EAR))); break;
	case LSRA: A = Blsr(A); break;
	case LSRB: B = Blsr(B); break;

	case ASR_idx: ofs=X+NextByte(); mc6800_memw(ofs, Basr(mc6800_memr(ofs))); break;
	case ASR: FetchAddr(); mc6800_memw(EAR, Basr(mc6800_memr(EAR))); break;
	case ASRA: A = Basr(A); break;
	case ASRB: B = Basr(B); break;

	case ASL_idx: ofs=X+NextByte(); mc6800_memw(ofs, Basl(mc6800_memr(ofs))); break;
	case ASL: FetchAddr(); mc6800_memw(EAR, Basl(mc6800_memr(EAR))); break;
	case ASLA: A = Basl(A); break;
	case ASLB: B = Basl(B); break;

	case ROR_idx: ofs=X+NextByte(); mc6800_memw(ofs, Bror(mc6800_memr(ofs))); break;
	case ROR: FetchAddr(); mc6800_memw(EAR, Bror(mc6800_memr(EAR))); break;
	case RORA: A = Bror(A); break;
	case RORB: B = Bror(B); break;

	case ROL_idx: ofs=X+NextByte(); mc6800_memw(ofs, Brol(mc6800_memr(ofs))); break;
	case ROL: FetchAddr(); mc6800_memw(EAR, Brol(mc6800_memr(EAR))); break;
	case ROLA: A = Brol(A); break;
	case ROLB: B = Brol(B); break;

	case BITA_imm: v = 0; TestByte(A & NextByte()); break;
	case BITA_dir: v = 0; TestByte(A & mc6800_memr(NextByte())); break;
	case BITA_idx: v = 0; TestByte(A & mc6800_memr(X + NextByte())); break;
	case BITA: FetchAddr(); v = 0; TestByte(A & mc6800_memr(EAR)); break;
	case BITB_imm: v = 0; TestByte(B & NextByte()); break;
	case BITB_dir: v = 0; TestByte(B & mc6800_memr(NextByte())); break;
	case BITB_idx: v = 0; TestByte(B & mc6800_memr(X + NextByte())); break;
	case BITB: FetchAddr(); v = 0; TestByte(B & mc6800_memr(EAR)); break;

	case CBA:	Bsub(A, B); break;

	case CMPA_imm: Bsub(A, NextByte()); break;
	case CMPA_dir: Bsub(A, mc6800_memr(NextByte())); break;
	case CMPA_idx: Bsub(A, mc6800_memr(X + NextByte())); break;
	case CMPA:	FetchAddr(); Bsub(A, mc6800_memr(EAR)); break;
	case CMPB_imm: Bsub(B, NextByte()); break;
	case CMPB_dir: Bsub(B, mc6800_memr(NextByte())); break;
	case CMPB_idx: Bsub(B, mc6800_memr(X + NextByte())); break;
	case CMPB:	FetchAddr(); Bsub(B, mc6800_memr(EAR)); break;

	case CPX_imm: FetchAddr(); Bcpx(X, EAR); break;
	case CPX_dir: ofs=NextByte(); r16=mc6800_memr(ofs)<<8; r16|=mc6800_memr(ofs+1); Bcpx(X, r16); break;
	case CPX_idx: ofs=NextByte()+X; r16=mc6800_memr(ofs)<<8; r16|=mc6800_memr(ofs+1); Bcpx(X, r16); break;
	case CPX: FetchAddr(); r16=mc6800_memr(EAR)<<8; r16|=mc6800_memr(EAR+1); Bcpx(X, r16); break;

	case TST_idx: c = v = 0; TestByte(mc6800_memr(X + NextByte())); break;
	case TST:	FetchAddr(); c = v = 0; TestByte(mc6800_memr(EAR)); break;
	case TSTA: c = v = 0; TestByte(A); break;
	case TSTB: c = v = 0; TestByte(B); break;

	case BCC: EAR=NextByte(); if (c==0) Branch(); break;
	case BCS: EAR=NextByte(); if (c==1) Branch(); break;
	case BEQ: EAR=NextByte(); if (z==1) Branch(); break;
	case BGE: EAR=NextByte(); if ((n^v)==0) Branch(); break;
	case BGT: EAR=NextByte(); if ((z|(n^v))==0) Branch(); break;
	case BHI: EAR=NextByte(); if ((c|z)==0) Branch(); break;
	case BLE: EAR=NextByte(); if ((z|(n^v))==1) Branch(); break;
	case BLS: EAR=NextByte(); if ((c|z)==1) Branch(); break;
	case BLT: EAR=NextByte(); if ((n^v)==1) Branch(); break;
	case BMI: EAR=NextByte(); if (n==1) Branch(); break;
	case BNE: EAR=NextByte(); if (z==0) Branch(); break;
	case BPL: EAR=NextByte(); if (n==0) Branch(); break;
	case BVC: EAR=NextByte(); if (v==0) Branch(); break;
	case BVS: EAR=NextByte(); if (v==1) Branch(); break;

	case BRA: EAR=NextByte(); Branch(); break;
	case BSR: EAR=NextByte(); mc6800_memw(SP--, PC&0xff); mc6800_memw(SP--, PC>>8); Branch(); break;

	case JMP_idx: PC = X + NextByte(); break;
	case JMP: FetchAddr(); PC = EAR; break;
	case JSR_idx: EAR = PC + 1; mc6800_memw(SP--, EAR&0xff); mc6800_memw(SP--, EAR>>8); PC=X+NextByte(); break;
	case JSR: FetchAddr(); mc6800_memw(SP--, PC&0xff); mc6800_memw(SP--, PC>>8); PC=EAR; break;

	case RTS: PC = mc6800_memr(++SP)<<8; PC |= mc6800_memr(++SP); break;

	case RTI:	t = mc6800_memr(++SP);
	    c=(t&1)!=0; v=(t&2)!=0; z=(t&4)!=0; n=(t&8)!=0; i=(t&16)!=0; h=(t&32)!=0;
	    B = mc6800_memr(++SP);
	    A = mc6800_memr(++SP);
	    X = mc6800_memr(++SP)<<8; X |= mc6800_memr(++SP);
	    PC = mc6800_memr(++SP)<<8; PC |= mc6800_memr(++SP);
	    break;

	case SWI:
	    if (SWIemulator(mc6800_memr(PC), &A, &B, &X, &t, &PC))
		PC++;
	    else {
		t = (c?1:0)|(v?2:0)|(z?4:0)|(n?8:0)|(i?16:0)|(h?32:0)|0xc0;
		mc6800_memw(SP--, PC&0xff); 
		mc6800_memw(SP--, PC>>8);
		mc6800_memw(SP--, X&0xff); 
		mc6800_memw(SP--, X>>8);
		mc6800_memw(SP--, A);
		mc6800_memw(SP--, B);
		mc6800_memw(SP--, t);
		PC = mc6800_memr(0xfffa)<<8; 
		PC |= mc6800_memr(0xfffb);
	    }
	    i = 1; 
	    break;

	case WAI:
	    i = fWai = 1;
	    break;
    }

    mc6800_global_takts += takt;

    return takt;
}
