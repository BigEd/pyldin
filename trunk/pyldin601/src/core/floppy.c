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

int floppyOn()
{
    return -1;
}

int floppyOff()
{
    return -1;
}

int floppyOp(int Op, int Drive, int Track, int Head, int Sector, UBYTE *mema)
{
    return 0xc0;
}

int readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst)
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

int writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src)
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

int formaTrack(int Disk, int Track, int Head)
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

int init765()
{
    if (!diskImage[FLOPPY_A])
	floppyOp(0, FLOPPY_A, 0, 0, 0, NULL);

    if (!diskImage[FLOPPY_B])
	floppyOp(0, FLOPPY_B, 0, 0, 0, NULL);

    return 0;
}
