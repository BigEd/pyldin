/*****************************************************************************
 *   target.c:  Target C file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/
#include "config.h"
#include <inttypes.h>
#include "irq.h"
#include "system.h"
#include "uart.h"
#include "leds.h"
#ifdef USE_LCD
#include "lcd.h"
#include "console/screen.h"
#endif
#ifdef USE_EMC
#include "emc.h"
#endif


void delayMs(unsigned long delayInMs)
{
    T0TCR = 0x02; // Stop and reset timer.
    T0PR  = 0x00; // Set prescaler to zero.
    T0MR0 = delayInMs * (Fpclk / 1000); // Set Value.
    T0IR  = 0xff; // Reset all interrrupt flags.
    T0MCR = 0x04; // Stop timer on match.
    T0TCR = 0x01; // Start timer.
    while (T0TCR & 0x01);
}

void Init_MAP(void)
{
#ifdef BOOT 
    MEMMAP = 0; // Boot loader Mode.     
#endif
#ifdef iFLASH 
    MEMMAP = 1; // User iFLASH Mode.     
#endif
#ifdef iSRAM 
    MEMMAP = 2; // User iSRAM Mode.
#endif
#ifdef xFLASH 
    MEMMAP = 3; // User External Memory Mode.
#endif

    MEMMAP = 1; // User iFLASH Mode.
}

void Init_CPC(void)
{
    uint32_t MValue, NValue; // M,N value.

    PCLKSEL0 = 0x55555555; // PCLK is 1/1 CCLK.
    PCLKSEL1 = 0x55555555;

    //PCLKSEL0 = 0xAAAAAAAA; // PCLK is 1/2 CCLK.
    //PCLKSEL1 = 0xAAAAAAAA;

    //PCLKSEL0 = 0x00000000; // PCLK is 1/4 CCLK.
    //PCLKSEL1 = 0x00000000;

    if (PLLSTAT & (1 << 25)) {
	PLLCON = 1; // Enable PLL, disconnected.
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
    }

    PLLCON = 0; // Disable PLL, disconnected.
    PLLFEED = 0xaa;
    PLLFEED = 0x55;

    SCS |= 0x20; // Enable main OSC.
    while(!(SCS & 0x40)); // Wait until main OSC is usable.

    CLKSRCSEL = 0x1; // Select main OSC, 12MHz, as the PLL clock source.

    PLLCFG = PLL_MValue | (PLL_NValue << 16);
    PLLFEED = 0xaa;
    PLLFEED = 0x55;

    PLLCON = 1; // Enable PLL, disconnected.
    PLLFEED = 0xaa;
    PLLFEED = 0x55;

    CCLKCFG = CCLKDivValue; // Set clock divider.
#if USE_USB
    USBCLKCFG = USBCLKDivValue; // Usbclk = 288 MHz/6 = 48 MHz.
#endif

    while (((PLLSTAT & (1 << 26)) == 0)); // Check lock bit status.

    MValue = PLLSTAT & 0x00007FFF;
    NValue = (PLLSTAT & 0x00FF0000) >> 16;

    while ((MValue != PLL_MValue) && ( NValue != PLL_NValue) );

    PLLCON = 3; // Enable and connect.
    PLLFEED = 0xaa;
    PLLFEED = 0x55;

    while ( ((PLLSTAT & (1 << 25)) == 0) ); // Check connect bit status.
}

void Init_MAM(void) 
{
    MAMCR = 0x00; // MAM functions disabled.
    MAMTIM = 0x03; // MAM fetch cycles are 3 CCLKs in duration.
    MAMCR = 0x02; // MAM functions full enabled.
}

void Init_GPIO(void)
{
    PINSEL0  = 0x00000000;
    PINSEL1  = 0x00000000;
    PINSEL2  = 0x00000000;
    PINSEL3  = 0x00000000;
    PINSEL4  = 0x00000000;
    PINSEL5  = 0x00000000;
    PINSEL6  = 0x00000000;
    PINSEL7  = 0x00000000;
    PINSEL8  = 0x00000000;
    PINSEL9  = 0x00000000;
    PINSEL10 = 0x00000000;

    IODIR0   = 0x00000000;
    IODIR1   = 0x00000000;
    //IO0SET   = 0x00000000;
    //IO1SET   = 0x00000000;

    FIO0DIR  = 0x00000000;
    FIO1DIR  = 0x00000000;
    FIO2DIR  = 0x00000000;
    FIO3DIR  = 0x00000000;
    FIO4DIR  = 0x00000000;

    //FIO0SET  = 0x00000000;
    //FIO1SET  = 0x00000000;
    //FIO2SET  = 0x00000000;
    //FIO3SET  = 0x00000000;
    //FIO4SET  = 0x00000000;

    FIO0MASK = 0x00000000;
    FIO1MASK = 0x00000000;
    FIO2MASK = 0x00000000;
    FIO3MASK = 0x00000000;
    FIO4MASK = 0x00000000;

    SCS |= 1; /* set GPIOx to use Fast I/O */
}

void Init_PORT(void)
{
    // GPIO:
    // UART0:
    CONFIG_PINSEL(PINSEL0, 2, 1);//  PINSEL0_bit.P0_2 = 0x01;
    CONFIG_PINSEL(PINSEL0, 3, 1);//  PINSEL0_bit.P0_3 = 0x01;
}

void systemSetup(void)
{
    Init_MAP();

    Init_CPC();

    Init_MAM();

    Init_GPIO();
#ifdef USE_EMC
    Init_EMC();
#endif
    Init_PORT();
#ifdef USE_LCD
    Init_LCD_controller();
#endif
    Init_VIC();

    Init_Leds();

    return;
}
