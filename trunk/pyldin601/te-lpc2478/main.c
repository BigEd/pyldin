#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "config.h"
#include "fio.h"
#include "uart.h"
#include "timer.h"

#include "core/mc6800.h"
#include "core/mc6845.h"
#include "core/devices.h"
#include "core/keyboard.h"
#include "core/floppy.h"
#include "core/printer.h"
#include "screen.h"

#define WAIT { int i; for(i=0; i<800000; i++) asm volatile (" nop "); }

#define LED_CONTROL(a, b, c) { \
    if (c)	\
	a |= (b); \
    else	\
	a &= ~(b);\
}

#define MEMORY_CPU	(LCD_BUFFER_ADDR + 0x30000)
#define MEMORY_RAMDRIVE	(MEMORY_CPU + 0x10000)

extern void mc6845_drawScreen_lpc24(void *video, int width, int height);

static int fReset = 0;

static int vscr_width = 320;
static int vscr_height = 240;
static int vScale = 1;

static uint16_t *pixels = (uint16_t *) LCD_BUFFER_ADDR;

void drawXbm(unsigned char *xbm, int xp, int yp, int w, int h, int over)
{
    screen_drawXbm(pixels, vscr_width, vscr_height, vScale, xbm, xp, yp, w, h, over);
}

void drawChar(unsigned int c, int xp, int yp, unsigned int fg, unsigned int bg)
{
    screen_drawChar(pixels, vscr_width, vscr_height, vScale, c, xp, yp, fg, bg);
}

void drawString(char *str, int xp, int yp, unsigned int fg, unsigned int bg)
{
    screen_drawString(pixels, vscr_width, vscr_height, vScale, str, xp, yp, fg, bg);
}

//
// Video ROM
//
static const uint8_t videorom[] = {
#include "video.rom.h"
};

byte *get_videorom_mem(dword size)
{
    return (byte *) videorom;
}

//
// BIOS ROM
//
static const uint8_t biosrom[] = {
#include "bios.rom.h"
};

byte *get_bios_mem(dword size)
{
    return (byte *) biosrom;
}

//
// ROM chips memory
//
static const uint8_t romchip0[] = {
#include "rom0.rom.h"
};
byte *get_romchip_mem(byte chip, dword size)
{
    switch(chip) {
    case 0:
	return (byte *) romchip0;
    }
    return NULL;
}

//
// CPU memory
//
byte *get_cpu_mem(dword size)
{
    return (byte *) MEMORY_CPU;
}

//
// RamDrive memory
//
byte *get_ramdisk_mem(dword size)
{
    return (byte *) MEMORY_RAMDRIVE;
}

//
// Printer emulation
//
void printer_put_char(byte data)
{
}

void Covox_Set(int val, int ticks)
{
}

void Speaker_Set(int val, int ticks)
{
}

//
// Keyboard emulation
//
void resetRequested(void)
{
    fReset = 1;
}

//
//
//
void clrScr(void)
{
    unsigned long  i;
    unsigned short *Ptr16;

    Ptr16 = (unsigned short *)LCD_BUFFER_ADDR;

    for(i = 0; i < 320 * 240; i++) {
	*Ptr16++ = 0x0;
    }
}

//
// 50Hz timer
//
void TimerHandler(void)
{
    static int vert = 0;

    devices_set_tick50();
    mc6845_curBlink();
    mc6800_setIrq(1);

    LED_CONTROL(BOARD_LED1_FIO, BOARD_LED1_MASK, vert & 1);
    LED_CONTROL(BOARD_LED2_FIO, BOARD_LED2_MASK, vert & 2);
    LED_CONTROL(BOARD_LED3_FIO, BOARD_LED3_MASK, vert & 4);
    vert++;
}

//
//
//
int main(void)
{
    FIOInit(BOARD_LED1_PORT, DIR_OUT, BOARD_LED1_MASK);
    FIOInit(BOARD_LED2_PORT, DIR_OUT, BOARD_LED2_MASK);
    FIOInit(BOARD_LED3_PORT, DIR_OUT, BOARD_LED3_MASK);

    LED_CONTROL(BOARD_LED1_FIO, BOARD_LED1_MASK, 0);
    LED_CONTROL(BOARD_LED2_FIO, BOARD_LED2_MASK, 0);
    LED_CONTROL(BOARD_LED3_FIO, BOARD_LED3_MASK, 0);

    vTimer0Init(20);

    clrScr();

    uart0Init(UART_BAUD(HOST_BAUD_U0), UART_8N1, UART_FIFO_8); // setup the UART

    uart0Puts("Pyldin-601 emulator system\r\n");

    mc6800_init();
    mc6800_reset();

    int vcounter = 0;		//
    int scounter = 0;		// syncro counter
    int takt;
    int cnt = 0;

    while (1) {
	takt = mc6800_step();

	vcounter += takt;
	scounter += takt;

	if (vcounter >= 20000) {
	    if ((cnt % 3) == 2)
		mc6845_drawScreen_lpc24(pixels, vscr_width, vscr_height);

	    vcounter = 0;

	    cnt++;
	}
	if (fReset == 1) {
	    mc6800_reset();
	    fReset = 0;
	}
    }

    return 0; 
}
