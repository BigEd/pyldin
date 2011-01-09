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

void INT17emulator(byte *A, byte *B, word *X, byte *t, word *PC);

#endif
