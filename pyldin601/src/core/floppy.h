/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#include "core/mc6800.h"

#define FLOPPY_A	0
#define FLOPPY_B	1

extern char *diskImage[];
extern int dSizes[];
extern int flopWrite[];

int floppy_status(int Disk);
int floppy_readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst);
int floppy_writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src);
int floppy_formaTrack(int Disk, int Track, int Head);
int floppy_init();

#endif
