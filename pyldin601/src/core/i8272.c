#include <stdio.h>
#include "core/mc6800.h"
#include "core/devices.h"
#include "core/i8272.h"
#include "core/floppy.h"

//#define DEBUG

static byte fdcslct = 0;
static byte fdcstat = 0;
static byte fdcdata = 0;

static byte cmdargs[16];
static byte cmdargc = 0;
static byte cmdcount = 0;

static byte retargs[16];
static byte retargc = 0;
static byte tmpretargc = 0;

static byte sector[512];
static int datacount;
static int readcount = 0;
static int writecount = 0;

static byte formatargs[4];
static int formatargc = 0;
static byte formatsect = 0;
static byte formatfill = 0;

static byte curtrack = 0;

void i8272_init(void)
{
    floppy_init();
}

#ifdef DEBUG
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
#endif

static void execute_command(void)
{
    int res;
#ifdef DEBUG
    int i;
    fprintf(stderr, "command %02X args ", fdcdata);
    for (i = 0; i < cmdcount; i++)
	fprintf(stderr, "%02X ", cmdargs[i]);
    fprintf(stderr, "\n");
#endif
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
	    retargs[1] = 0x20; /* | (floppy_diskReady((fdcslct & 4)?0:1)?0:0x40); */
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
	    if (fdcdata == 0x45)
		res = floppy_status((fdcslct & 4)?0:1);
	    else
		res = floppy_readSector((fdcslct & 4)?0:1, cmdargs[1], cmdargs[3], cmdargs[2], sector);
	    if (res)
		break;
	    if (fdcdata == 0x45)
		writecount = 0x80 << cmdargs[4];
	    if (fdcdata == 0x66)
		readcount = 0x80 << cmdargs[4];
	    datacount = 0;
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
#ifdef DEBUG
	    fprintf(stderr, "FORMAT sectors %d fill %02X\n", formatsect, formatfill);
#endif
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
#ifdef DEBUG
	fprintf(stderr, "write ");
	show_fdc_select();
#endif
	if (!(fdcslct & 1)) {
	    formatsect = 0;
	    writecount = 0;
	    readcount = 0;
	    retargc = 0;
	}
	return;
    case 0x10:
	// read-only register
	return;
    case 0x11:
	if (formatsect) {
	    formatargs[formatargc++] = d;
	    if (formatargc == 4) {
		formatsect--;
		formatargc = 0;
		datacount = 0;
		int size = 0x80 << formatargs[3];
		while (size--)
		    sector[datacount++] = formatfill;
#ifdef DEBUG
		fprintf(stderr, "FORMAT %d %d %d %d (%d)\n", formatargs[0], formatargs[1], formatargs[2], formatargs[3], size);
#endif
		if (floppy_writeSector((fdcslct & 4)?0:1, formatargs[0], formatargs[2], formatargs[1], sector)) {
		    retargs[6] = 0x40;
		    retargs[5] = 0x35;
		    break;
		}
	    }
	    break;
	}
	if (writecount) {
	    writecount--;
	    sector[datacount++] = d;
	    if (!writecount) {
		if (floppy_writeSector((fdcslct & 4)?0:1, retargs[3], retargs[1], retargs[2], sector)) {
		    retargs[6] = 0x40;
		    retargs[5] = 0x35;
		}
	    }
	    break;
	}
#ifdef DEBUG
	fprintf(stderr, "write DATA=%02X\n", d);
#endif
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
#ifndef ROMOPTIMIZE
	default:
	    fprintf(stderr, "FDC I8272: Unknown command %02X\n", d);
#endif
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
#ifdef DEBUG
	fprintf(stderr, "read  ");
	show_fdc_select();
#endif
	return fdcslct;
    case 0x10:
	return fdcstat | 0x80 | (retargc?0x40:0) | (readcount?0x40:0);
    case 0x11:
	if (readcount) {
	    readcount--;
	    return sector[datacount++];
	}
	if (retargc) {
#ifdef DEBUG
	    fprintf(stderr, "return %02X\n", retargs[retargc - 1]);
#endif
	    return retargs[--retargc];
	}
	return fdcdata;
    }

    return 0xff;
}
