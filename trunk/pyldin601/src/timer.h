/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

extern int	fRef;
extern int	fTick;
extern int	tick50;
extern int	curBlink;
extern void installTimerHandler(void);
extern void removeTimerHandler(void);
