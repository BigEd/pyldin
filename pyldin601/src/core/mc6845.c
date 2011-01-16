#include <stdio.h>
#include <string.h>
#include "core/mc6800.h"
#include "core/mc6845.h"

static int curBlink = 0;
static byte video_regs[16];
static byte iomap[4];

static byte resolution = 0;
static byte vMode = 0;

static short txt260 = -1;
static short grf260 = -1;

static byte *videorom = NULL;

static byte old_rHor = 0;
static byte old_rVer = 0;

O_INLINE void mc6845_curBlink(void)
{
    curBlink++;
}

O_INLINE void mc6845_writeReg(byte a, byte d)
{
    video_regs[a & 0xf] = d;
}

O_INLINE byte mc6845_readReg(byte a)
{
    return video_regs[a & 0xf];
}

O_INLINE void mc6845_write(byte a, byte d)
{
    a &= 1;
    iomap[a] = d;
    if (a == 1)
	mc6845_writeReg(iomap[0], iomap[1]);
}

O_INLINE byte mc6845_read(byte a)
{
    a &= 1;
    if (a == 1)
	return mc6845_readReg(iomap[0]);
    return iomap[a];
}

void mc6845_init(void)
{
    videorom = get_videorom_mem(2048);
}

O_INLINE void mc6845_setupScreen(int mode)
{
    vMode = mode & 0x20;
    resolution = mode & 0x02;
}

void mc6845_drawScreen(void *video, int width, int height, int scale)
{
    dword c, v, j, i, ofj, ofi;
    word *vmem = (word *) video;
    byte *mem = mc6800_get_memory();
    byte *src = mem;
    byte *crsr = mem;

    byte cur_start = mc6845_readReg(0x0a) & 0x1f;
    byte cur_start1 = mc6845_readReg(0x0a);
    byte cB_1 = mc6845_readReg(0x0a) & 0x60;
    byte cur_end = mc6845_readReg(0x0b) & 0x1f;

    if (cur_end > 7)
	cur_end = 7;
    if ((cur_start > cur_end) ||
	(cur_start1 == 0x20) ||
	(curBlink > 25 && cB_1 != 0)) {
	    cur_start = 8;
	    cur_end = 7;
    }
    if (curBlink > 50)
	curBlink = 0;

    byte rHor = mc6845_readReg(0x01);
    byte rVer = mc6845_readReg(0x06);

    if ((rHor != old_rHor) ||
	(rVer != old_rVer))
	memset(video, 0, width * (height - 24 * scale) * 2);

    old_rHor = rHor;
    old_rVer = rVer;

    if ((rVer == 0) || (rHor == 0))
	return;

    if (vMode == 0) {
	src += (word)((mc6845_readReg(0x0c) << 8) + mc6845_readReg(0x0d));
	crsr += (word)((mc6845_readReg(0x0e) << 8) + mc6845_readReg(0x0f) + txt260);

	if (rHor > 42)
	    rHor = 42;
	if (rVer > 29)
	    rVer = 29;

	ofj = ((width - ((rHor > 40)?40:rHor) * 8 * scale) >> 1) + ((height - 24 * scale - rVer * 8 * scale) >> 1) * width;

	for (j = 0; j < rVer; j++) {
	    for (i = 0; i < rHor; i++) {
		ofi=ofj + i * 8 * scale;
		c = *src++;
		c = ((c << 1) | (c >> 7)) & 0xff;
		c = c * 8;
		if (i < 40) 
		    for (v = 0; v < 8; v++) {
			unsigned int vY;
			for (vY = 0; vY < scale; vY++) {
			    unsigned int z;
			    byte t1 = videorom[c + v];
			    unsigned short *vscr1 = vmem + ofi;
			    if (crsr == src)
				    if (v>=cur_start && v<=cur_end) 
					t1^=0xff;
			    for (z=0; z<8; z++) {
				unsigned int z1;
				unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
				for (z1 = 0; z1 < scale; z1++)
				    *vscr1++ = tt;
				t1 <<= 1;
			    }
			    ofi += width;
			}
		    }
		if (src >= mem + 0xffff) 
		    src = mem + 0xf000;
	    }
	    ofj += width * 8 * scale;
	}
    } else {
	src += (word)(((mc6845_readReg(0x0c) << 8) + mc6845_readReg(0x0d)) << 3);
	crsr += (word)(((mc6845_readReg(0x0e) << 8) + mc6845_readReg(0x0f) + grf260) << 3);

	if (rHor > 48)
	    rHor = 48;
	if (rVer > 28)
	    rVer = 28;

	ofj = ((width - ((rHor > 40)?40:rHor) * 8 * scale) >> 1) + ((height - 24 * scale - rVer * 8 * scale) >> 1) * width;

	for (j = 0; j < rVer; j++) {
	    for (i = 0; i < rHor; i++) {
		ofi = ofj + i * 8 * scale;
		if (src >= mem + 0xfff8)
		    src = mem;
		if (crsr != src) {
		    for (v = 0; v < 8; v++) {
			unsigned int vY;
			for (vY = 0; vY < scale; vY++) {
			    unsigned int z;
			    byte t1 = *src;
			    unsigned short *vscr1 = vmem + ofi;
			    for (z = 0; z < 8; z++) {
				unsigned int z1;
				unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
				for (z1 = 0; z1 < scale; z1++)
				    *vscr1++ = tt;
				t1 <<= 1;
			    }
			    ofi += width;
			}
			src++;
		    }
		} else {
		    for (v = 0; v < 8; v++) {
			unsigned int vY;
			for (vY = 0; vY < scale; vY++) {
			    unsigned int z;
			    byte t1;
			    if ((v >= cur_start) && (v <= cur_end))
				t1 = *src ^ 0xff;
			    else
				t1 = *src;
			    unsigned short *vscr1 = vmem + ofi;
			    for (z = 0; z < 8; z++) {
				unsigned int z1;
				unsigned short tt = (t1 & 0x80)?PIXEL_ON:PIXEL_OFF;
				for (z1 = 0; z1 < scale; z1++)
				    *vscr1++ = tt;
				t1 <<= 1;
			    }
			    ofi += width;
			}
			src++;
		    }
		}
	    }
	    ofj += width * 8 * scale;
	}
    }
}
