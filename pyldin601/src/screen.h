/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#ifndef _SCREEN_H_
#define _SCREEN_H_

void screen_drawXbm(void *video, int width, int height, int scale, unsigned char *xpm, int xp, int yp, int w, int h, int over);
void screen_drawChar(void *video, int width, int height, int scale, unsigned int c, int xp, int yp, unsigned int fg, unsigned int bg);
void screen_drawString(void *video, int width, int height, int scale, char *str, int xp, int yp, unsigned int fg, unsigned int bg);

#endif
