#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "config.h"
#include "fio.h"
#include "uart.h"
#include "timer.h"
#include "kbd.h"
#include "unlzma.h"
#include "diskovod.h"

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

#define SPEAKER_CONTROL LED_CONTROL

#define MEMORY_CPU		(LCD_BUFFER_ADDR + 0x30000)
#define MEMORY_CPU_SIZE		0x10000
#define MEMORY_VIDEOROM		(MEMORY_CPU + MEMORY_CPU_SIZE)
#define MEMORY_VIDEOROM_SIZE	0x800
#define MEMORY_BIOS		(MEMORY_VIDEOROM + MEMORY_VIDEOROM_SIZE)
#define MEMORY_BIOS_SIZE	0x1000
#define MEMORY_ROMCHIPS		(MEMORY_BIOS + MEMORY_BIOS_SIZE)
#define MEMORY_ROMCHIPS_SIZE	(0x10000 * MAX_ROMCHIPS)
#define MEMORY_RAMDRIVE		(MEMORY_ROMCHIPS + MEMORY_ROMCHIPS_SIZE)
#define MEMORY_RAMDRIVE_SIZE	0x80000

extern void core_50Hz_irq(void);
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
// unlzma error call
//
void unlzma_error(char *x)
{
    uart0Puts("unlzma: ");
    uart0Puts(x);
    uart0Puts("\r\n");
}

//
// Video ROM
//
static const byte videorom[] = {
#include "video.lzma.h"
};

byte *get_videorom_mem(dword size)
{
    size = size;

    unlzma((unsigned char *)videorom, sizeof(videorom), NULL, NULL, (byte *)MEMORY_VIDEOROM, NULL, unlzma_error);

    return (byte *) MEMORY_VIDEOROM;
}

//
// BIOS ROM
//
static const byte biosrom[] = {
#include "bios.lzma.h"
};

byte *get_bios_mem(dword size)
{
    size = size;

    unlzma((unsigned char *)biosrom, sizeof(biosrom), NULL, NULL, (byte *)MEMORY_BIOS, NULL, unlzma_error);

    return (byte *) MEMORY_BIOS;
}

//
// ROM chips memory
//
static const byte romchip0[] = {
#include "rom0.lzma.h"
};

static const byte romchip1[] = {
#include "rom1.lzma.h"
};

static const byte romchip2[] = {
#include "rom2.lzma.h"
};

static const byte romchip3[] = {
#include "rom3.lzma.h"
};

static const byte romchip4[] = {
#include "rom4.lzma.h"
};

static const struct {
    const byte *rom;
    dword size;
} romchips[] = {
    {romchip0, sizeof(romchip0)},
    {romchip1, sizeof(romchip1)},
    {romchip2, sizeof(romchip2)},
    {romchip3, sizeof(romchip3)},
    {romchip4, sizeof(romchip4)}
};

byte *get_romchip_mem(byte chip, dword size)
{
    size = size;

    if (chip < MAX_ROMCHIPS) {
	unlzma((unsigned char *)romchips[chip].rom, romchips[chip].size, NULL, NULL, (byte *) (MEMORY_ROMCHIPS + 0x10000 * chip), NULL, unlzma_error);
	return (byte *) (MEMORY_ROMCHIPS + 0x10000 * chip);
    }

    return NULL;
}

//
// CPU memory
//
byte *get_cpu_mem(dword size)
{
    size = size;

    return (byte *) MEMORY_CPU;
}

//
// RamDrive memory
//
byte *get_ramdisk_mem(dword size)
{
    size = size;

    return (byte *) MEMORY_RAMDRIVE;
}

//
// Printer emulation
//
void printer_put_char(byte data)
{
    data = data;
}

void Covox_Set(int val, int ticks)
{
    val = val;
    ticks = ticks;
}

void Speaker_Set(int val, int ticks)
{
    ticks = ticks;
    SPEAKER_CONTROL(PYLDIN_SPEAKER_FIO, PYLDIN_SPEAKER_MASK, val);
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
void timer_handler(void)
{
    core_50Hz_irq();
    floppy_power_timeout();
}

//
// Keyboard interrupt
//
void keyboard_handler(uint8_t scancode)
{
    uint8_t code = scancode & 0x7f;
    if (scancode & 0x80) {
	if (code < 0x60)
	    jkeybUp();
	else {
	    switch (code) {
	    case KEY_CONTROL:	jkeybModeUp(1); break;
	    case KEY_SHIFT:	jkeybModeUp(2); break;
	    }
	}
    } else {
	if (code < 0x60)
	    jkeybDown(code);
	else {
	    switch (code) {
	    case KEY_CONTROL:	jkeybModeDown(1); break;
	    case KEY_SHIFT:	jkeybModeDown(2); break;
	    case KEY_PAUSE:	resetRequested(); break;
	    }
	}
    }
}

//
// led control
//
void led_control(int led, int v)
{
    switch (led) {
    case 0:
	LED_CONTROL(BOARD_LED1_FIO, BOARD_LED1_MASK, v);
	break;
    case 1:
	LED_CONTROL(BOARD_LED2_FIO, BOARD_LED2_MASK, v);
	break;
    case 2:
	LED_CONTROL(BOARD_LED3_FIO, BOARD_LED3_MASK, v);
	break;
    }
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

    FIOInit(PYLDIN_SPEAKER_PORT, DIR_OUT, PYLDIN_SPEAKER_MASK);
    SPEAKER_CONTROL(PYLDIN_SPEAKER_FIO, PYLDIN_SPEAKER_MASK, 0);

    timer_init(20);

#ifndef USE_USBKEY
    keyboard_init();
#endif

    clrScr();

    uart0Init(UART_BAUD(HOST_BAUD_U0), UART_8N1, UART_FIFO_8); // setup the UART

    uart0Puts("Pyldin-601 emulator system\r\n");

#ifdef USE_USBKEY
    Host_Init();
    if (Host_EnumDev()) {
	uart0Puts("USB Keyboard inited\n");
    }
#endif

#ifdef MEMINFO
    {
    char buf[128];
    sprintf(buf, "Allocated %d bytes of SDRAM\r\n", MEMORY_RAMDRIVE + MEMORY_RAMDRIVE_SIZE - LCD_BUFFER_ADDR);
    uart0Puts(buf);
    }
#endif
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
#ifdef USE_USBKEY
	    HID_InputKeyboard();
#endif
	    if ((cnt % 3) == 2)
#ifdef USE_LCD1BPP
		mc6845_drawScreen_lpc24_1bpp(pixels, vscr_width, vscr_height);
#else
		mc6845_drawScreen_lpc24(pixels, vscr_width, vscr_height);
#endif
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
