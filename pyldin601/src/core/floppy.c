/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
typedef unsigned char UBYTE;
#include "floppy.h"

#define SSIZE	512
#define NSECT	9
#define NHEAD	2

char *diskImage[] = {
    NULL, NULL, NULL, NULL
};

int dSizes[] = {
    0, 0, 0, 0
};

int flopWrite[] = {
    0, 0, 0, 0
};

static int floppyOp(int Op, int Drive, int Track, int Head, int Sector, UBYTE *mema)
{
    return 0xc0;
}

static int readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst)
{
    if (Track > 79 || Sector > 18) 
	return 0x40;

    if (!diskImage[Disk])
	return floppyOp(1, Disk, Track, Head, Sector, dst);

    int offs;

    if (dSizes[Disk] > 737280) 
        offs = (Sector-1)*SSIZE+Head*18*SSIZE+Track*18*NHEAD*SSIZE;
    else 
	offs = (Sector-1)*SSIZE+Head*NSECT*SSIZE+Track*NSECT*NHEAD*SSIZE;

    memcpy(dst, diskImage[Disk] + offs, SSIZE);

    return 0;
}

static int writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src)
{
    if (Track > 79 || Sector > 18) 
	return 0x40;

    if (!diskImage[Disk])
	return floppyOp(2, Disk, Track, Head, Sector, src);

    flopWrite[Disk] = 1;

    int offs;

    if (dSizes[Disk] > 737280) 
	offs = (Sector-1)*SSIZE+Head*18*SSIZE+Track*18*NHEAD*SSIZE;
    else 
	offs = (Sector-1)*SSIZE+Head*NSECT*SSIZE+Track*NSECT*NHEAD*SSIZE;

    memcpy(diskImage[Disk] + offs, src, SSIZE);
    
    return 0;
}

static int formaTrack(int Disk, int Track, int Head)
{
    if (Track > 79) 
	return 0x40;

    if (!diskImage[Disk])
	return floppyOp(4, Disk, Track, Head, 0, NULL);

    flopWrite[Disk] = 1;

    int offs;

    if (dSizes[Disk] > 737280) 
	offs = Head*18*SSIZE+Track*18*NHEAD*SSIZE;
    else 
	offs = Head*NSECT*SSIZE+Track*NSECT*NHEAD*SSIZE;

    if (dSizes[Disk] > 737280) 
	memset(diskImage[Disk] + offs, 0xf6, 18*SSIZE);
    else 
	memset(diskImage[Disk] + offs, 0xe5, NSECT*SSIZE);

    return 0;
}

static int init765()
{
    if (!diskImage[FLOPPY_A])
	floppyOp(0, FLOPPY_A, 0, 0, 0, NULL);

    if (!diskImage[FLOPPY_B])
	floppyOp(0, FLOPPY_B, 0, 0, 0, NULL);

    return 0;
}


void INT17emulator(byte *A, byte *B, word *X, byte *t, word *PC)
{
    byte sect_buf[512];
    word bukva, i2;

    int devs  = mc6800_memr(*X) & 0x01;
    int track = mc6800_memr(*X + 1);
    int head  = mc6800_memr(*X + 2) & 0x01;
    int sect  = mc6800_memr(*X + 3);
    int offs  = (mc6800_memr(*X + 4) << 8) | mc6800_memr(*X + 5);

    bukva = mc6800_memr(0xed20) << 8; 
    bukva |= mc6800_memr(0xed21);

//    if (m601a == 0) 
	bukva += 81; 
//    else 
//	bukva += 159;

    switch(*A) {
	case 0x80:
	case 0:
	    *A = init765(); 
	    break;

	case 1:
	    mc6800_memw(bukva, 0x52);
	    *A = readSector(devs, track, sect, head, sect_buf);
	    for (i2 = 0; i2 < 512; i2++)
		mc6800_memw(offs + i2, sect_buf[i2]);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 2:	
	    mc6800_memw(bukva, 0x57);
	    for (i2 = 0; i2 < 512; i2++)
		sect_buf[i2] = mc6800_memr(offs + i2);
	    *A = writeSector(devs, track, sect, head, sect_buf);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 3:	
	    mc6800_memw(bukva, 0x53);
	    *A = readSector(devs, track, sect, head, sect_buf);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 4:	
	    mc6800_memw(bukva, 0x46);
	    *A = formaTrack(devs, track, head);
	    mc6800_memw(bukva, 0x20);
	    break;

	default: 
	    *A = 0;
	    break;
    }
}
