#include <stdio.h>
#include "core/mc6800.h"
#include "core/devices.h"
#include "core/i8272.h"
#include "core/floppy.h"

static byte fdcslct = 0;
static byte fdcstat = 0;
static byte fdcdata = 0;

static byte cmdargs[16];
static byte cmdargc = 0;
static byte cmdcount = 0;

static byte retargs[16];
static byte retargc = 0;
static byte tmpretargc = 0;

static byte *secptr = NULL;
static int readcount = 0;
static int writecount = 0;

static byte formatargs[4];
static int formatargc = 0;
static byte formatsect = 0;
static byte formatfill = 0;

static byte curtrack = 0;

void i8272_init(void)
{
}

static void show_fdc_select(void)
{
    if (fdcslct & 1)
	fprintf(stderr, "-res=1 ");
    else
	fprintf(stderr, "-res=0 ");

    if (fdcslct & 2)
	fprintf(stderr, "TC=1 ");
    else
	fprintf(stderr, "TC=0 ");

    if (fdcslct & 4)
	fprintf(stderr, "drive=1 ");
    else
	fprintf(stderr, "drive=0 ");

    if (fdcslct & 8)
	fprintf(stderr, "motor=1\n");
    else
	fprintf(stderr, "motor=0\n");

}

static void execute_command(void)
{
    int i;
    fprintf(stderr, "command %02X args ", fdcdata);
    for (i = 0; i < cmdcount; i++)
	fprintf(stderr, "%02X ", cmdargs[i]);
    fprintf(stderr, "\n");
    switch (fdcdata) {
	case 0x04:
	    retargs[0] = 0x20;
	    retargc = 1;
	    break;
	case 0x07:
	    retargs[0] = 0;
	    retargs[1] = 0x20;
	    tmpretargc = 2;
	    curtrack = 0;
	    break;
	case 0x0f:
	    retargs[0] = cmdargs[1];
	    retargs[1] = 0x20;
	    tmpretargc = 2;
	    curtrack = cmdargs[1];
	    break;
	case 0x4a:
	    retargs[0] = 0;
	    retargs[1] = 0;
	    retargs[2] = 0;
	    retargs[3] = curtrack;
	    retargs[4] = 0;
	    retargs[5] = 0;
	    retargs[6] = 0xc0;
	    retargc = 7;
	    break;
	case 0x45:
	case 0x66:
	    secptr = floppy_getSector((fdcslct & 4)?0:1, cmdargs[1], cmdargs[3], cmdargs[2]);
	    if (!secptr)
		break;
	    if (fdcdata == 0x45)
		writecount = 0x80 << cmdargs[4];
	    if (fdcdata == 0x66)
		readcount = 0x80 << cmdargs[4];
	    retargs[0] = cmdargs[4];
	    retargs[1] = cmdargs[3];
	    retargs[2] = cmdargs[2];
	    retargs[3] = cmdargs[1];
	    retargs[4] = 0;
	    retargs[5] = 0;
	    retargs[6] = 0xc0;
	    retargc = 7;
	    curtrack = cmdargs[1];
	    break;
	case 0x4d:
	    formatargc = 0;
	    formatsect = cmdargs[2];
	    formatfill = cmdargs[4];
	    fprintf(stderr, "FORMAT sectors %d fill %02X\n", formatsect, formatfill);
	    retargs[0] = cmdargs[1];
	    retargs[1] = 0;
	    retargs[2] = 0;
	    retargs[3] = 0;
	    retargs[4] = 0;
	    retargs[5] = 0;
	    retargs[6] = 0xc0;
	    retargc = 7;
	    curtrack = cmdargs[1];
	    break;
	case 0x08:
	    retargc = tmpretargc;
	    tmpretargc = 0;
	    break;
    }
}

void i8272_write(byte a, byte d)
{
    a &= 0x1f;
    switch (a) {
    case 0x0:
	fdcslct = d;
	fprintf(stderr, "write ");
	show_fdc_select();
	if (!(fdcslct & 1)) {
	    formatsect = 0;
	    writecount = 0;
	    readcount = 0;
	    retargc = 0;
	}
	return;
    case 0x10:
	fdcstat = d;
	return;
    case 0x11:
	if (formatsect) {
	    formatargs[formatargc++] = d;
	    if (formatargc == 4) {
		formatsect--;
		formatargc = 0;
		secptr = floppy_getSector((fdcslct & 4)?0:1, formatargs[0], formatargs[2], formatargs[1]);
		int size = 0x80 << formatargs[3];
		fprintf(stderr, "FORMAT %d %d %d %d (%d)\n", formatargs[0], formatargs[1], formatargs[2], formatargs[3], size);
		while (size--)
		    *secptr++ = formatfill;
	    }
	    break;
	}
	if (writecount) {
	    writecount--;
	    //fprintf(stderr, "write [%04X:%02X]\n", writecount, d);
	    *secptr++ = d;
	    break;
	}
	fprintf(stderr, "write DATA=%02X\n", d);
	if (cmdargc) {
	    cmdargs[cmdcount++] = d;
	    if (!--cmdargc)
		execute_command();
	    break;
	} else
	    fdcdata = d;
	cmdcount = 0;
	switch (d) {
	case 0x03: // specify
	    cmdargc = 2;
	    break;
	case 0x04:
	    cmdargc = 1;
	    break;
	case 0x07: // recalibrate
	    cmdargc = 1;
	    break;
	case 0x08: // Sense Interrupt Status
	    execute_command();
	    break;
	case 0x0f: // seek
	    cmdargc = 2;
	    break;
	case 0x4a: // read id
	    cmdargc = 1;
	    break;
	case 0x45: // write
	case 0x66: // read
	    cmdargc = 8;
	    break;
	case 0x4d: // format
	    cmdargc = 5;
	    break;
	default:
	    fprintf(stderr, "------>Unknown command %d\n", d);
	}
	return;
    }

    return;
}

byte i8272_read(byte a)
{
    a &= 0x1f;

    switch (a) {
    case 0x0:
	fprintf(stderr, "read  ");
	show_fdc_select();
	return fdcslct;
    case 0x10:
	return fdcstat | 0x80 | (retargc?0x40:0) | (readcount?0x40:0);
    case 0x11:
	if (readcount) {
	    readcount--;
	    //fprintf(stderr, "return [%04X:%02X]\n", readcount, *secptr);
	    return *secptr++;
	}
	if (retargc) {
	    fprintf(stderr, "return %02X\n", retargs[retargc - 1]);
	    return retargs[--retargc];
	}
	return fdcdata;
    }

    return 0xff;
}
