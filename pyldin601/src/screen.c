/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define SCREEN_DEPTH	2
#define SCREEN_WIDTH_OFS 0

#define PIXEL_ON		(0x3f<<5) //0xffff
#define PIXEL_OFF		0x0

#define VPIXEL_ON		(0x1f<<5) //
#define KBD_COLOR		0xffff //((0x3f<<5) | 0x1f)

#include "virtkbd.xbm"
#include "vmenu.xbm"

int vkbdEnabled = 0;
int redrawVMenu = 0;
int clearVScr = 0;

//byte vregs[16];
extern byte *vMem;

byte m601a = 0;
short txt260 = -1;
short grf260 = -1;

static byte resolution, vMode = 0, old_vMode = 0;

static byte font[2048];

word *vscr;		//videomem start
int vscr_width = 640;
int vscr_height = 480;
int vScale = 2;

void setupScr(int mode)
{
	vMode = mode & 0x20;
//	if (vMode!=old_vMode) ClrLines(0,37119);
	old_vMode = vMode;
	resolution = mode & 0x02;
	clearVScr = 1;
	redrawVMenu = 1;
//	palette = (mode & 0x18)>>3;
//	colorMode = mode & 0x04;
}

void drawVMenu(void)
{
	int ofj = (((vscr_height - SCREEN_HEIGHT * vScale) >> 1) + 216 * vScale) * vscr_width + ((vscr_width - SCREEN_WIDTH * vScale) >> 1);
	int ifj = 0;
	int i, j, x;
	for (j = 0; j < vmenu_height; j++) {
		int vY;
		for (vY = 0; vY < vScale; vY++) {
		    for (i = 0; i < (vmenu_width>>3); i++) {
			unsigned char c = vmenu_bits[ifj + i];
			for (x = 0; x < 8; x++) {
			    int z1;
			    for (z1 = 0; z1 < vScale; z1++) {
				vscr[ofj + (i * 8 + x) * vScale + z1] = (c & 0x1)?0:0x3ef;
			    }
			    c >>= 1;
			}
		    }
		    ofj += vscr_width;
		}
		ifj += (vmenu_width>>3);
	}
}

void clearScr(void)
{
	memset((char *)vscr, 0, vscr_width * vscr_height * 2);
}

void refreshScr()
{
	byte *src = vMem;
	byte *crsr = vMem;
	dword c, v, j, i, ofj, ofi;

	if (clearVScr) {
		clearScr();
		clearVScr = 0;
	}

	byte cur_start = vregs[0x0a] & 0x1f;
	byte cur_start1 = vregs[0x0a];
	byte cB_1 = vregs[0x0a] & 0x60;
	byte cur_end = vregs[0x0b] & 0x1f;

	if (cur_end > 7) cur_end = 7;
	if ((cur_start > cur_end) || (cur_start1 == 0x20)
			|| (curBlink > 25 && cB_1 != 0)) {
		cur_start = 8;
		cur_end = 7;
	}
	if (curBlink > 50) curBlink = 0;

	if (vMode == 0) {
		src += (word)((vregs[0x0c] << 8) + vregs[0x0d]);
		crsr += (word)((vregs[0x0e] << 8) + vregs[0x0f] + txt260);
		byte rHor = vregs[0x01];
		byte rVer = vregs[0x06];
		if (rHor > 42) rHor=42;
		if (rVer > 29) rVer=29;

		// для коррекции вывода 27 строки при BIOS 2.60
		// if (txt260 == 1 && rVer > 26) rVer=29;

		ofj = ((vscr_width - SCREEN_WIDTH * vScale) >> 1) + ((vscr_height - SCREEN_HEIGHT * vScale) >> 1) * vscr_width;
		if (vkbdEnabled == 0) { //no virtual keyboard on screen
			for (j = 0; j < rVer; j++) {
			    for (i = 0; i < rHor; i++) {
					ofi=ofj + i * 8 * vScale;
					c = *src++;
					c = ((c << 1) | (c >> 7)) & 0xff;
					c = c * 8;
					if (i < 40) 
					for (v = 0; v < 8; v++) {
					    unsigned int vY;
					    for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = font[c + v];
						unsigned short *vscr1 = vscr + ofi;
						if (crsr == src) 
						    if (v>=cur_start && v<=cur_end) 
							t1^=0xff;
						for (z=0; z<8; z++) {
						    unsigned int z1;
						    unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
						    for (z1 = 0; z1 < vScale; z1++) {
						        *vscr1++ = tt;
						    }
						    t1 <<= 1;
						}
						ofi += vscr_width;
					    }
					}
					if (src >= vMem + 0xffff) 
					    src = vMem + 0xf000;
			    }
			    ofj += vscr_width * 8 * vScale;
			}
		} else {
			unsigned int vkbd_y = 0;
			unsigned int vkbd_yt = 0;
			for (j = 0; j < rVer; j++) {
			    for (i = 0; i < rHor; i++) {
					ofi=ofj + i * 8 * vScale;
					vkbd_yt = vkbd_y+i;
					c = *src++;
					c = ((c<<1) | (c>>7)) & 0xff;
					c = c * 8;
					if (i<40) 
					for (v = 0; v < 8; v++) {
					    unsigned int vY;
					    for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = font[c + v];
						byte vt1 = virtkbd_pict_bits[vkbd_yt];
						unsigned short *vscr1 = vscr + ofi;
						if (crsr == src) 
						    if (v>=cur_start && v<=cur_end) 
							t1^=0xff;
						for (z=0; z<8; z++) {
						    unsigned int z1;
						    unsigned short tt = ((t1 & 0x80)?VPIXEL_ON:PIXEL_OFF) | ((vt1 & 0x1)?0:KBD_COLOR);
						    for (z1 = 0; z1 < vScale; z1++) {
						        *vscr1++ = tt;
						    }
						    t1 <<= 1;
						    vt1 >>= 1;
						}
						ofi += vscr_width;
					    }
					    vkbd_yt += (virtkbd_pict_width>>3);
					}
					if (src >= vMem + 0xffff) 
					    src = vMem + 0xf000;
			    }
			    ofj += vscr_width * 8 * vScale;
			    vkbd_y += (virtkbd_pict_width >>3 ) * 8;
			}
		}
	} else {
		src += (word)(((vregs[0x0c] << 8) + vregs[0x0d]) << 3);
		crsr += (word)(((vregs[0x0e] << 8) + vregs[0x0f] + grf260) << 3);
		byte rHor = vregs[0x01];
		byte rVer = vregs[0x06];
		if (rHor > 48) rHor=48;
		if (rVer > 28) rVer=28;

		ofj = ((vscr_width - SCREEN_WIDTH * vScale) >> 1) + ((vscr_height - SCREEN_HEIGHT * vScale) >> 1) * vscr_width;

		if (vkbdEnabled == 0) {
/* drawing without virtual keyboard */
		for (j = 0; j < rVer; j++) {
		    for (i = 0; i < rHor; i++) {
				ofi = ofj + i * 8 * vScale;
				if (src >= vMem + 0xfff8) src = vMem;
				if (crsr != src) {
				    for (v = 0; v < 8; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
				    }
				} else {
				    for (v = 0; v < cur_start; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
				    }
				    for (v = cur_start; v <= cur_end; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src ^ 0xff;
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
				    }
				    for (v = cur_end+1; v < 8; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
				    }
				}		
		    }
		    ofj += vscr_width * 8 * vScale;
		}
/* end drawing without virtual keyboard */
		} else {
/* drawing with virtual keyboard */
		unsigned int vkbd_y = 0;
		unsigned int vkbd_yt = 0;
		for (j = 0; j < rVer; j++) {
		    for (i = 0; i < rHor; i++) {
				ofi = ofj + i * 8 * vScale;
				vkbd_yt = vkbd_y + i;
				if (src >= vMem + 0xfff8) src = vMem;
				if (crsr != src) {
				    for (v = 0; v < 8; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						byte vt1 = virtkbd_pict_bits[vkbd_yt];
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = ((t1 & 0x80)?VPIXEL_ON:PIXEL_OFF) | ((vt1 & 0x1)?0:KBD_COLOR);
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						    vt1 >>= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
					vkbd_yt += (virtkbd_pict_width>>3);
				    }
				} else {
				    for (v = 0; v < cur_start; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						byte vt1 = virtkbd_pict_bits[vkbd_yt];
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = ((t1 & 0x80)?VPIXEL_ON:PIXEL_OFF) | ((vt1 & 0x1)?0:KBD_COLOR);
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						    vt1 >>= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
					vkbd_yt += (virtkbd_pict_width>>3);
				    }
				    for (v = cur_start; v <= cur_end; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src ^ 0xff;
						byte vt1 = virtkbd_pict_bits[vkbd_yt];
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = ((t1 & 0x80)?VPIXEL_ON:PIXEL_OFF) | ((vt1 & 0x1)?0:KBD_COLOR);
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						    vt1 >>= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
					vkbd_yt += (virtkbd_pict_width>>3);
				    }
				    for (v = cur_end+1; v < 8; v++) {
					unsigned int vY;
					for (vY = 0; vY < vScale; vY++) {
						unsigned int z;
						byte t1 = *src;
						byte vt1 = virtkbd_pict_bits[vkbd_yt];
						unsigned short *vscr1 = vscr + ofi;
						for (z = 0; z < 8; z++) {
						    unsigned int z1;
						    unsigned short tt = ((t1 & 0x80)?VPIXEL_ON:PIXEL_OFF) | ((vt1 & 0x1)?0:KBD_COLOR);
						    for (z1 = 0; z1 < vScale; z1++) {
							*vscr1++ = tt;
						    }
						    t1 <<= 1;
						    vt1 >>= 1;
						}
						//if (enable320 == 0) if (i==39) t1=0;
						ofi += vscr_width;
					}
					src++;
					vkbd_yt += (virtkbd_pict_width>>3);
				    }
				}		
			}
		    ofj += vscr_width * 8 * vScale;
		    vkbd_y += (virtkbd_pict_width >> 3) * 8;
		}
/* end drawing with virtual keyboard */
		}

	}
	if (redrawVMenu) {
		redrawVMenu = 0;
		drawVMenu();
	}
}

int loadTextFont(char *name)
{
	gzFile inf = gzopen(name, "rb");
	if (!inf) return -1;
	gzread(inf, font, 2048);
	gzclose(inf);
	return 0;
}

void drawChar(int x, int y, dword c, word fg, word bg)
{
	int i, j;
	dword offset = ((vscr_width - SCREEN_WIDTH * vScale) >> 1) + x * vScale + 
			((vscr_height - SCREEN_HEIGHT * vScale) >> 1) * vscr_width + y * vscr_width * vScale;
	
	c = ((c<<1) | (c>>7)) & 0xff;
	c = c * 8;
	for (j = 0; j < 8; j++) {
	    int vY;
	    for (vY = 0; vY < vScale; vY++) {
		byte d = font[c + j];
		for (i=0; i < 8; i++) {
		    int z1;
		    for (z1 = 0; z1 < vScale; z1++) {
			vscr[offset + i * vScale + z1] = (d & 0x80)?fg:bg;
		    }
		    d<<=1;
		}
		offset += vscr_width;
	    }
	}
}

void drawString(int x, int y, char *str, word fg, word bg)
{
	y*=8;
	while (*str != 0) {
	    drawChar(x, y, *str, fg, bg);
	    x+=8;
	    str++;
	}
}
