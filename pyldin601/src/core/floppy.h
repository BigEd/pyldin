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

#define SSIZE	512
#define NSECT	9
#define NHEAD	2

extern char *diskImage[];
extern int dSizes[];
extern int flopWrite[];

byte *floppy_getSector(int Disk, int Track, int Sector, int Head);

void INT17emulator(byte *A, byte *B, word *X, byte *t, word *PC);

#endif
