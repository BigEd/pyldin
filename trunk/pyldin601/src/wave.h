/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

extern int Speaker_Init(void);
extern void Speaker_Finish(void);
extern void Covox_Set(int val, int ticks);
extern void Speaker_Set(int val, int ticks);
extern void Speaker_Clear(int ticks);
