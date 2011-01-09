/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
#if defined(__APPLE__) && (__GNUC__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <time.h>

#include "mc6800.h"
#include "opcode.h"
#include "mpu_cycles.h"

#include "keyboard.h"
//#include "timer.h"
//#include "screen.h"
#include "floppy.h"
//#include "wave.h"
#include "printer.h"

#ifdef __GNUC__
#define INLINE inline
#endif

static	byte	*MEM;
static	byte	*BMEM;
static	byte	*vdiskMEM;

#define MAX_ROMCHIPS	5
static	byte	*ROMP[MAX_ROMCHIPS];	// 5 x 64KB - max. 320KB
static	byte	*CurrP;			// указатель на содержимое текущей страницы

static	word	fWai;			// установлен после WAI
static	byte	fSpeaker;		// бит состояния динамика

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

byte vregs[16];
byte *vMem;

#define memr	mc6800_memr
#define memw	mc6800_memw

extern void setupScr(int mode);
extern void Speaker_Set(int val, int ticks);

dword	mc6800_global_takts;

extern	int	tick50;	// устанавливается в 1 при TIMER INT 50Hz

int IRQrequest = 0;

byte wr_mem[512];

dword vdiskAddress;

dword vdiskSIZE = 524288;

static	void	INT17emulator(); // эмуляция управления контроллером диска

int mc6800_init(void)
{
    int i;
    mc6800_global_takts = 0;

    MEM  	= (byte *) malloc(sizeof(byte) * 65538);
    BMEM 	= (byte *) malloc(sizeof(byte) * 4096 );
    vdiskMEM 	= (byte *) malloc(sizeof(byte) * vdiskSIZE);
    vMem 	= MEM;

    memset(MEM, 0, 65535);

    for (i = 0; i < MAX_ROMCHIPS; i++) {
	ROMP[i] = (byte *) malloc(sizeof(byte) * 65536);
	memset(ROMP[i], 0xff, 65536);
    }

    CurrP = &MEM[0xc000];
    
    return 0;
}

int mc6800_fini(void)
{
    int i;

    free(MEM);
    free(BMEM);
    free(vdiskMEM);

    for (i = 0; i < MAX_ROMCHIPS; i++) 
	free(ROMP[i]);
    
    return 0;
}

/*INLINE*/ byte mc6800_memr(word a)
{
    byte t;

    if (a >= 0xf000)
	return BMEM[a - 0xf000]; //чтение системного BIOS

    if (a >= 0xc000 && a < 0xe000) 
	return CurrP[a - 0xc000]; //чтение ROMpage

    if (a == 0xe628) 
	return readKbd(); //чтение порта a (клавиатуры)

    if (a == 0xe62a || a==0xe62e)	//чтение упр.порта a
	return checkKbd() | ((led_status & KBD_LED_CAPS)?0:8) | 0x37;

    if (a == 0xe62b) {	//чтение упр.порта b
	t = tick50 | fSpeaker | 0x37;
	tick50 = 0;
	return t;
    }

    if (a == 0xe6d0)
	return 0x80;

    if (a == 0xe683) {
	t = vdiskMEM[vdiskAddress % vdiskSIZE];
	vdiskAddress += 1; 
	vdiskAddress %= vdiskSIZE;
	return t;
    }

    if (a == 0xe632) 
	return 0x80;	//для принтера 601А

    if (a == 0xe634)
	return printer_dra_rd();

    return MEM[a];
}

/*INLINE*/ void mc6800_memw(word a, byte d)
{
    byte old_3s=0;

    if (a == 0xe6f0) {
	if (d & 8) {
//	    if ((d > 15)) 
//		fprintf(stderr, "rompage = %02x (PC=%04X)\n", d, PC);

	    int chip = (d >> 4) % MAX_ROMCHIPS;
	    int page = d & 7;
	    
	    CurrP = ROMP[chip] + page * 8192; //запись в рег.номера страниц
	} else {
	    CurrP = &MEM[0xc000];
	}
    }

    if (a == 0xe601 || a == 0xe605) 
	vregs[MEM[a-1] & 0xf] = d; //запись данных в рег.видеоконтроллера
	
    if (a == 0xe629) {
	MEM[0xe62d] = d; //только для программы kltr.ubp
	cyrMode = (d&1)?0:4;
	setupScr(d);
    }
    
    if (a == 0xe62a || a==0xe62e) {
	if (d & 0x08) 
	    led_status = led_status & ~KBD_LED_CAPS;
	else 
	    led_status = led_status | KBD_LED_CAPS;
    }

    if (a == 0xe62b || (a == 0xe635)) {
	//запись в упр.рег b
	//и COVOX, если разрешена его эмуляция
	old_3s=fSpeaker;
	fSpeaker = d & 0x08;
	if (old_3s != fSpeaker) 
	    Speaker_Set(fSpeaker, mc6800_global_takts);
	old_3s = fSpeaker;
    }

    if (a == 0xe680) 
	vdiskAddress = (vdiskAddress & 0x0ffff) | ((d & 0x0f)<<16);

    if (a == 0xe681) 
	vdiskAddress = (vdiskAddress & 0xf00ff) | (d<<8);

    if (a == 0xe682) 
	vdiskAddress = (vdiskAddress & 0xfff00) | d;

    if (a == 0xe683) {
	vdiskMEM[vdiskAddress % vdiskSIZE] = d;
	vdiskAddress += 1; 
	vdiskAddress %= vdiskSIZE;
    }

    if (a == 0xe635)
	printer_drb_wr(d);

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

static INLINE void Bcpx(int a, int b)
{
    int wans = a - b;
    int ans = wans & 0xffff;

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
    return memr(PC++);
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
	memw(SP--, PC&0xff); memw(SP--, PC>>8);
	memw(SP--, X&0xff); memw(SP--, X>>8);
	memw(SP--, A);
	memw(SP--, B);
	memw(SP--, t);
	PC = memr(0xfff8)<<8; PC |= memr(0xfff9);
	IRQrequest = 0;
	mc6800_global_takts += 12;
	return 12;
    }

    byte opnum = memr(PC++);

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

	case PSHA:	memw(SP--, A); break;
	case PSHB:	memw(SP--, B); break;
	case PULA:	A = memr(++SP); break;
	case PULB:	B = memr(++SP); break;

	case DEC_idx:	ofs=NextByte()+X; oc=c; memw(ofs,Bsub(memr(ofs),1)); c=oc; break;
	case DEC:	FetchAddr(); oc=c; memw(EAR, Bsub(memr(EAR),1)); c=oc; break;
	case DECA:	oc=c; A=Bsub(A,1); c=oc; break;
	case DECB:	oc=c; B=Bsub(B,1); c=oc; break;
	case DES:	SP--; break;
	case DEX:	X--; z = X?0:1; break;

	case INC_idx:	ofs=NextByte()+X; oh=h; oc=c; memw(ofs,Badd(memr(ofs),1)); c=oc; h=oh; break;
	case INC:	FetchAddr(); oh=h; oc=c; memw(EAR, Badd(memr(EAR),1)); c=oc; h=oh; break;
	case INCA:	oh=h; oc=c; A=Badd(A,1); c=oc; h=oh; break;
	case INCB:	oh=h; oc=c; B=Badd(B,1); c=oc; h=oh; break;
	case INS:	SP++; break;
	case INX:	X++; z = X?0:1; break;

	case CLR_idx:	memw(NextByte()+X, 0); n = v = c = 0; z = 1; break;
	case CLR:	FetchAddr(); memw(EAR, 0); n = v = c = 0; z = 1; break;
	case CLRA:	A = n = v = c = 0; z = 1; break;
	case CLRB:	B = n = v = c = 0; z = 1; break;

	case COM_idx: ofs = NextByte()+X; memw(ofs, ~memr(ofs)); c = 1; v = 0; TestByte(memr(ofs)); break;
	case COM:	FetchAddr(); memw(EAR, ~memr(EAR)); c = 1; v = 0; TestByte(memr(EAR)); break;
	case COMA:	A = ~A; c = 1; v = 0; TestByte(A); break;
	case COMB:	B = ~B; c = 1; v = 0; TestByte(B); break;

	case NEG_idx: ofs = NextByte()+X; memw(ofs, Bsub(0, memr(ofs))); break;
	case NEG:	FetchAddr(); memw(EAR, Bsub(0, memr(EAR))); break;
	case NEGA:	A = Bsub(0, A); break;
	case NEGB:	B = Bsub(0, B); break;

	case LDAA_imm: A = NextByte(); v = 0; TestByte(A); break;
	case LDAA_dir: A = memr(NextByte()); v = 0; TestByte(A); break;
	case LDAA_idx: A = memr(X + NextByte()); v = 0; TestByte(A); break;
	case LDAA:	FetchAddr(); A = memr(EAR); v = 0; TestByte(A); break;
	case LDAB_imm: B = NextByte(); v = 0; TestByte(B); break;
	case LDAB_dir: B = memr(NextByte()); v = 0; TestByte(B); break;
	case LDAB_idx: B = memr(X + NextByte()); v = 0; TestByte(B); break;
	case LDAB:	FetchAddr(); B = memr(EAR); v = 0; TestByte(B); break;

	case LDS_imm: FetchAddr(); SP = EAR; v = 0; TestWord(SP); break;
	case LDS_dir: ofs=NextByte(); SP = memr(ofs) << 8; SP |= memr(ofs + 1); v = 0; TestWord(SP); break;
	case LDS_idx: ofs=X+NextByte(); SP = memr(ofs) << 8; SP |= memr(ofs + 1); v = 0; TestWord(SP); break;
	case LDS:	FetchAddr(); SP = memr(EAR) << 8; SP |= memr(EAR + 1); v = 0; TestWord(SP); break;
	case LDX_imm: FetchAddr(); X = EAR; v = 0; TestWord(X); break;
	case LDX_dir: ofs=NextByte(); X = memr(ofs) << 8; X |= memr(ofs + 1); v = 0; TestWord(X); break;
	case LDX_idx: ofs=X+NextByte(); X=memr(ofs)<<8; X|=memr(ofs+1); v = 0; TestWord(X); break;
	case LDX:	FetchAddr(); X = memr(EAR) << 8; X |= memr(EAR + 1); v = 0; TestWord(X); break;

	case STAA_dir: memw(NextByte(), A); v = 0; TestByte(A); break;
	case STAA_idx: memw(X + NextByte(), A); v = 0; TestByte(A); break;
	case STAA:	FetchAddr(); memw(EAR, A); v = 0; TestByte(A); break;
	case STAB_dir: memw(NextByte(), B); v = 0; TestByte(B); break;
	case STAB_idx: memw(X + NextByte(), B); v = 0; TestByte(B); break;
	case STAB:	FetchAddr(); memw(EAR, B); v = 0; TestByte(B); break;

	case STS_dir: ofs=NextByte(); memw(ofs,SP>>8); memw(ofs+1, SP&0xff); v=0; TestWord(SP); break;
	case STS_idx: ofs=X+NextByte(); memw(ofs,SP>>8); memw(ofs+1,SP&0xff); v=0; TestWord(SP); break;
	case STS:	FetchAddr(); memw(EAR,SP>>8); memw(EAR+1,SP&0xff); v=0; TestWord(SP); break;
	case STX_dir: ofs=NextByte(); memw(ofs,X>>8); memw(ofs+1, X&0xff); v=0; TestWord(X); break;
	case STX_idx: ofs=X+NextByte(); memw(ofs,X>>8); memw(ofs+1,X&0xff); v=0; TestWord(X); break;
	case STX:	FetchAddr(); memw(EAR,X>>8); memw(EAR+1,X&0xff); v=0; TestWord(X); break;

	case ABA:	A = Badd(A, B); break;

	case ADCA_imm: A = Baddc(A, NextByte()); break;
	case ADCA_dir: A = Baddc(A, memr(NextByte())); break;
	case ADCA_idx: A = Baddc(A, memr(X + NextByte())); break;
	case ADCA:	FetchAddr(); A = Baddc(A, memr(EAR)); break;
	case ADCB_imm: B = Baddc(B, NextByte()); break;
	case ADCB_dir: B = Baddc(B, memr(NextByte())); break;
	case ADCB_idx: B = Baddc(B, memr(X + NextByte())); break;
	case ADCB:	FetchAddr(); B = Baddc(B, memr(EAR)); break;

	case ADDA_imm: A = Badd(A, NextByte()); break;
	case ADDA_dir: A = Badd(A, memr(NextByte())); break;
	case ADDA_idx: A = Badd(A, memr(X + NextByte())); break;
	case ADDA:	FetchAddr(); A = Badd(A, memr(EAR)); break;
	case ADDB_imm: B = Badd(B, NextByte()); break;
	case ADDB_dir: B = Badd(B, memr(NextByte())); break;
	case ADDB_idx: B = Badd(B, memr(X + NextByte())); break;
	case ADDB:	FetchAddr(); B = Badd(B, memr(EAR)); break;

	case SBA:	A = Bsub(A, B); break;

	case SBCA_imm: A = Bsubc(A, NextByte()); break;
	case SBCA_dir: A = Bsubc(A, memr(NextByte())); break;
	case SBCA_idx: A = Bsubc(A, memr(X + NextByte())); break;
	case SBCA:	FetchAddr(); A = Bsubc(A, memr(EAR)); break;
	case SBCB_imm: B = Bsubc(B, NextByte()); break;
	case SBCB_dir: B = Bsubc(B, memr(NextByte())); break;
	case SBCB_idx: B = Bsubc(B, memr(X + NextByte())); break;
	case SBCB:	FetchAddr(); B = Bsubc(B, memr(EAR)); break;

	case SUBA_imm: A = Bsub(A, NextByte()); break;
	case SUBA_dir: A = Bsub(A, memr(NextByte())); break;
	case SUBA_idx: A = Bsub(A, memr(X + NextByte())); break;
	case SUBA:	FetchAddr(); A = Bsub(A, memr(EAR)); break;
	case SUBB_imm: B = Bsub(B, NextByte()); break;
	case SUBB_dir: B = Bsub(B, memr(NextByte())); break;
	case SUBB_idx: B = Bsub(B, memr(X + NextByte())); break;
	case SUBB:	FetchAddr(); B = Bsub(B, memr(EAR)); break;

	case ANDA_imm: A &= NextByte(); v = 0; TestByte(A); break;
	case ANDA_dir: A &= memr(NextByte()); v = 0; TestByte(A); break;
	case ANDA_idx: A &= memr(X + NextByte()); v = 0; TestByte(A); break;
	case ANDA:	FetchAddr(); A &= memr(EAR); v = 0; TestByte(A); break;
	case ANDB_imm: B &= NextByte(); v = 0; TestByte(B); break;
	case ANDB_dir: B &= memr(NextByte()); v = 0; TestByte(B); break;
	case ANDB_idx: B &= memr(X + NextByte()); v = 0; TestByte(B); break;
	case ANDB:	FetchAddr(); B &= memr(EAR); v = 0; TestByte(B); break;

	case ORAA_imm: A |= NextByte(); v = 0; TestByte(A); break;
	case ORAA_dir: A |= memr(NextByte()); v = 0; TestByte(A); break;
	case ORAA_idx: A |= memr(X + NextByte()); v = 0; TestByte(A); break;
	case ORAA:	FetchAddr(); A |= memr(EAR); v = 0; TestByte(A); break;
	case ORAB_imm: B |= NextByte(); v = 0; TestByte(B); break;
	case ORAB_dir: B |= memr(NextByte()); v = 0; TestByte(B); break;
	case ORAB_idx: B |= memr(X + NextByte()); v = 0; TestByte(B); break;
	case ORAB:	FetchAddr(); B |= memr(EAR); v = 0; TestByte(B); break;

	case EORA_imm: A ^= NextByte(); v = 0; TestByte(A); break;
	case EORA_dir: A ^= memr(NextByte()); v = 0; TestByte(A); break;
	case EORA_idx: A ^= memr(X + NextByte()); v = 0; TestByte(A); break;
	case EORA:	FetchAddr(); A ^= memr(EAR); v = 0; TestByte(A); break;
	case EORB_imm: B ^= NextByte(); v = 0; TestByte(B); break;
	case EORB_dir: B ^= memr(NextByte()); v = 0; TestByte(B); break;
	case EORB_idx: B ^= memr(X + NextByte()); v = 0; TestByte(B); break;
	case EORB:	FetchAddr(); B ^= memr(EAR); v = 0; TestByte(B); break;

	case LSR_idx: ofs=X+NextByte(); memw(ofs, Blsr(memr(ofs))); break;
	case LSR: FetchAddr(); memw(EAR, Blsr(memr(EAR))); break;
	case LSRA: A = Blsr(A); break;
	case LSRB: B = Blsr(B); break;

	case ASR_idx: ofs=X+NextByte(); memw(ofs, Basr(memr(ofs))); break;
	case ASR: FetchAddr(); memw(EAR, Basr(memr(EAR))); break;
	case ASRA: A = Basr(A); break;
	case ASRB: B = Basr(B); break;

	case ASL_idx: ofs=X+NextByte(); memw(ofs, Basl(memr(ofs))); break;
	case ASL: FetchAddr(); memw(EAR, Basl(memr(EAR))); break;
	case ASLA: A = Basl(A); break;
	case ASLB: B = Basl(B); break;

	case ROR_idx: ofs=X+NextByte(); memw(ofs, Bror(memr(ofs))); break;
	case ROR: FetchAddr(); memw(EAR, Bror(memr(EAR))); break;
	case RORA: A = Bror(A); break;
	case RORB: B = Bror(B); break;

	case ROL_idx: ofs=X+NextByte(); memw(ofs, Brol(memr(ofs))); break;
	case ROL: FetchAddr(); memw(EAR, Brol(memr(EAR))); break;
	case ROLA: A = Brol(A); break;
	case ROLB: B = Brol(B); break;

	case BITA_imm: v = 0; TestByte(A & NextByte()); break;
	case BITA_dir: v = 0; TestByte(A & memr(NextByte())); break;
	case BITA_idx: v = 0; TestByte(A & memr(X + NextByte())); break;
	case BITA: FetchAddr(); v = 0; TestByte(A & memr(EAR)); break;
	case BITB_imm: v = 0; TestByte(B & NextByte()); break;
	case BITB_dir: v = 0; TestByte(B & memr(NextByte())); break;
	case BITB_idx: v = 0; TestByte(B & memr(X + NextByte())); break;
	case BITB: FetchAddr(); v = 0; TestByte(B & memr(EAR)); break;

	case CBA:	Bsub(A, B); break;

	case CMPA_imm: Bsub(A, NextByte()); break;
	case CMPA_dir: Bsub(A, memr(NextByte())); break;
	case CMPA_idx: Bsub(A, memr(X + NextByte())); break;
	case CMPA:	FetchAddr(); Bsub(A, memr(EAR)); break;
	case CMPB_imm: Bsub(B, NextByte()); break;
	case CMPB_dir: Bsub(B, memr(NextByte())); break;
	case CMPB_idx: Bsub(B, memr(X + NextByte())); break;
	case CMPB:	FetchAddr(); Bsub(B, memr(EAR)); break;

	case CPX_imm: FetchAddr(); Bcpx(X, EAR); break;
	case CPX_dir: ofs=NextByte(); r16=memr(ofs)<<8; r16|=memr(ofs+1); Bcpx(X, r16); break;
	case CPX_idx: ofs=NextByte()+X; r16=memr(ofs)<<8; r16|=memr(ofs+1); Bcpx(X, r16); break;
	case CPX: FetchAddr(); r16=memr(EAR)<<8; r16|=memr(EAR+1); Bcpx(X, r16); break;

	case TST_idx: c = v = 0; TestByte(memr(X + NextByte())); break;
	case TST:	FetchAddr(); c = v = 0; TestByte(memr(EAR)); break;
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
	case BSR: EAR=NextByte(); memw(SP--, PC&0xff); memw(SP--, PC>>8); Branch(); break;

	case JMP_idx: PC = X + NextByte(); break;
	case JMP: FetchAddr(); PC = EAR; break;
	case JSR_idx: EAR = PC + 1; memw(SP--, EAR&0xff); memw(SP--, EAR>>8); PC=X+NextByte(); break;
	case JSR: FetchAddr(); memw(SP--, PC&0xff); memw(SP--, PC>>8); PC=EAR; break;

	case RTS: PC = memr(++SP)<<8; PC |= memr(++SP); break;

	case RTI:	t = memr(++SP);
	    c=(t&1)!=0; v=(t&2)!=0; z=(t&4)!=0; n=(t&8)!=0; i=(t&16)!=0; h=(t&32)!=0;
	    B = memr(++SP);
	    A = memr(++SP);
	    X = memr(++SP)<<8; X |= memr(++SP);
	    PC = memr(++SP)<<8; PC |= memr(++SP);
	    break;

	case SWI:
	    if (SWIemulator(memr(PC), &A, &B, &X, &t, &PC))
		PC++;
	    else if (memr(PC) == 0x17) {
		PC++;
		INT17emulator();
	    } else {
		t = (c?1:0)|(v?2:0)|(z?4:0)|(n?8:0)|(i?16:0)|(h?32:0)|0xc0;
		memw(SP--, PC&0xff); 
		memw(SP--, PC>>8);
		memw(SP--, X&0xff); 
		memw(SP--, X>>8);
		memw(SP--, A);
		memw(SP--, B);
		memw(SP--, t);
		PC = memr(0xfffa)<<8; 
		PC |= memr(0xfffb);
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

static void INT17emulator()
{
    word bukva, i2;

    int devs  = memr(X) & 0x01;
    int track = memr(X+1);
    int head  = memr(X+2) & 0x01;
    int sect  = memr(X+3);
    int offs  = (memr(X+4) << 8) | memr(X+5);

    bukva = MEM[0xed20]<<8; 
    bukva |= MEM[0xed21];

//    if (m601a == 0) 
	bukva += 81; 
//    else 
//	bukva += 159;

    switch(A) {
	case 0x80:
	case 0: 
	    A = init765(); 
	    break;

	case 1:	
	    MEM[bukva] = 0x52;
	    A = readSector(devs, track, sect, head, MEM + offs);
	    MEM[bukva] = 0x20; 
	    break;

	case 2:	
	    MEM[bukva] = 0x57;
	    for (i2=0; i2<512; i2++) 
		wr_mem[i2]=memr(offs+i2);
	    A = writeSector(devs, track, sect, head, wr_mem);
	    MEM[bukva] = 0x20; 
	    break;

	case 3:	
	    MEM[bukva] = 0x53;
	    A = readSector(devs, track, sect, head, wr_mem);
	    MEM[bukva] = 0x20; 
	    break;

	case 4:	
	    MEM[bukva] = 0x46;
	    A = formaTrack(devs, track, head);
	    MEM[bukva] = 0x20; 
	    break;

	default: 
	    A = 0; 
	    break;
    }
}


byte *mc6800_getRomPtr(int chip, int page)
{
    if (page >= 0) {
	chip = chip % MAX_ROMCHIPS;
	page = page & 7;
	return ROMP[chip] + page * 8192;
    } else 
	return &BMEM[0];
}

void mc6800_reset()
{
    i = fWai = tick50 = IRQrequest = 0;
    PC = memr(0xfffe)<<8; 
    PC |= memr(0xffff);
}

void mc6800_setDATETIME(word year, word mon, word mday, word hour, word min, word sec)
{
    MEM[0x1c] = mday;
    MEM[0x1d] = mon + 1;
    
    year = (year % 100) + 1000 * (1 + year / 100);
    
    MEM[0x1e] = year >> 8;
    MEM[0x1f] = year % 256;
    
    MEM[0x18] = 0;
    MEM[0x19] = sec;
    MEM[0x1a] = min;
    MEM[0x1b] = hour;

    MEM[0xed00]=0xa5;
    MEM[0xed01]=0x5a;
}
