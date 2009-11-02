/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#ifndef __MC6845_H__
#define __MC6845_H__

#define byte	unsigned char
#define word	unsigned short
#define dword	unsigned int

extern int vScale;
extern byte vregs[16];
extern byte *vMem;
extern word *vscr;
extern byte m601a;
extern short txt260;
extern short grf260;
extern int vkbdEnabled;
extern int redrawVMenu;
extern int clearVScr;

void setupScr(int mode);
void clearScr(void);
extern void drawVMenu(void);
extern void refreshScr();
extern void refreshScr_A();
int loadTextFont(char *name);
extern void viewInfo();

void drawChar(int x, int y, dword c, word fg, word bg);
void drawString(int x, int y, char *str, word fg, word bg);

#endif
