/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#ifndef __FLOPPY_H__
#define __FLOPPY_H__

#define FLOPPY_A	0
#define FLOPPY_B	1

extern char *diskImage[];
extern int dSizes[];
extern int flopWrite[];

extern int floppyOn();
extern int floppyOff();
extern int loadDiskImage(char *name, int disk);
extern int unloadDiskImage(char *name, int disk);
extern int readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst);
extern int writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src);
extern int formaTrack(int Disk, int Track, int Head);
extern int init765();

#endif
