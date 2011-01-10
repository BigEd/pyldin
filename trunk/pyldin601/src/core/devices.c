#include <stdio.h>
#include <unistd.h>
#include "core/mc6800.h"
#include "core/devices.h"
#include "core/keyboard.h"
#include "printer.h"

static	byte	*BMEM;
static	byte	*vdiskMEM;

static	byte	*ROMP[MAX_ROMCHIPS];	// 5 x 64KB - max. 320KB
static	byte	*CurrP;			// указатель на содержимое текущей страницы

static dword vdiskAddress;
static dword vdiskSIZE = 524288;

static byte vregs_addr = 0;
byte vregs[16];
byte *vMem;

static byte led_status = 0;

static int tick50;	// устанавливается в 1 при TIMER INT 50Hz

static byte fSpeaker;		// бит состояния динамика

extern void setupScr(int mode);
extern void Speaker_Set(int val, int ticks);

int devices_init(byte *MEM)
{
    int i;

    BMEM 	= (byte *) get_bios_mem(4096); //malloc(sizeof(byte) * 4096 );
    vdiskMEM 	= (byte *) get_ramdisk_mem(vdiskSIZE); //malloc(sizeof(byte) * vdiskSIZE);
    vMem 	= MEM;

    for (i = 0; i < MAX_ROMCHIPS; i++) {
	ROMP[i] = (byte *) get_romchip_mem(i, 65536); //malloc(sizeof(byte) * 65536);
    }

    CurrP = NULL;

    return 0;
}

int devices_fini(void)
{
    return 0;
}

void devices_reset(void)
{
    tick50 = 0;
}

void devices_setDATETIME(word year, word mon, word mday, word hour, word min, word sec)
{
    mc6800_memw(0x1c, mday);
    mc6800_memw(0x1d, mon + 1);

    year = (year % 100) + 1000 * (1 + year / 100);

    mc6800_memw(0x1e, year >> 8);
    mc6800_memw(0x1f, year % 256);

    mc6800_memw(0x18, 0);
    mc6800_memw(0x19, sec);
    mc6800_memw(0x1a, min);
    mc6800_memw(0x1b, hour);

    mc6800_memw(0xed00, 0xa5);
    mc6800_memw(0xed01, 0x5a);
}

int devices_memr(word a, byte *t)
{

    if (a >= 0xf000) {
	*t = BMEM[a - 0xf000]; //чтение системного BIOS
	return 1;
    }

    if (a >= 0xc000 && a < 0xe000) {
	if (!CurrP)
	    return 0;
	*t = CurrP[a - 0xc000]; //чтение ROMpage
	return 1;
    }

    if ((a & 0xff00) != 0xe600)
	return 0;

    switch (a) {
    case 0xe600:
    case 0xe604:
	*t = vregs_addr; // чтение адреса регистра видеоконтроллера
	return 1;

    case 0xe601:
    case 0xe605:
	*t = vregs[vregs_addr & 0xf]; //чтение данных из рег.видеоконтроллера
	return 1;

    case 0xe628:
	*t = readKbd(); //чтение порта a (клавиатуры)
	return 1;

    case 0xe62a:
    case 0xe62e:	//чтение упр.порта a
	*t = checkKbd() | ((led_status & KBD_LED_CAPS)?0:8) | 0x37;
	return 1;

    case 0xe62b:	//чтение упр.порта b
	*t = tick50 | fSpeaker | 0x37;
	tick50 = 0;
	return 1;

    case 0xe6d0:
	*t = 0x80;
	return 1;

    case 0xe683:
	*t = vdiskMEM[vdiskAddress % vdiskSIZE];
	vdiskAddress += 1; 
	vdiskAddress %= vdiskSIZE;
	return 1;

    case 0xe632:
	*t = 0x80;	//для принтера 601А
	return 1;

    case 0xe634:
	*t = printer_dra_rd();
	return 1;
    }

    return 0;
}

int devices_memw(word a, byte d)
{
    if ((a & 0xff00) != 0xe600)
	return 0;

    byte old_3s=0;

    switch (a) {
    case 0xe6f0:
	if (d & 8) {
	    int chip = (d >> 4) % MAX_ROMCHIPS;
	    int page = d & 7;
	    CurrP = ROMP[chip] + page * 8192; //запись в рег.номера страниц
	} else {
	    CurrP = NULL;
	}
	return 0;

    case 0xe600:
    case 0xe604:
	vregs_addr = d; // запись адреса регистра видеоконтроллера
	return 0;

    case 0xe601:
    case 0xe605:
	vregs[vregs_addr & 0xf] = d; //запись данных в рег.видеоконтроллера
	return 0;

    case 0xe629:
	mc6800_memw(0xe62d, d); //только для программы kltr.ubp
	setCyrMode((d&1)?0:4);
	setupScr(d);
	return 0;

    case 0xe62a:
    case 0xe62e:
	if (d & 0x08) 
	    led_status = led_status & ~KBD_LED_CAPS;
	else 
	    led_status = led_status | KBD_LED_CAPS;
	return 0;

    case 0xe62b:
	//запись в упр.рег b
	//и COVOX, если разрешена его эмуляция
	old_3s=fSpeaker;
	fSpeaker = d & 0x08;
	if (old_3s != fSpeaker) 
	    Speaker_Set(fSpeaker, mc6800_get_takts());
	old_3s = fSpeaker;
	return 0;

    case 0xe635:
	printer_drb_wr(d);
	return 0;

    case 0xe680:
	vdiskAddress = (vdiskAddress & 0x0ffff) | ((d & 0x0f)<<16);
	return 0;

    case 0xe681:
	vdiskAddress = (vdiskAddress & 0xf00ff) | (d<<8);
	return 0;

    case 0xe682:
	vdiskAddress = (vdiskAddress & 0xfff00) | d;
	return 0;

    case 0xe683:
	vdiskMEM[vdiskAddress % vdiskSIZE] = d;
	vdiskAddress += 1;
	vdiskAddress %= vdiskSIZE;
	return 0;
    }

    return 0;
}

void devices_set_tick50(void)
{
    tick50 = 0x80;
}
