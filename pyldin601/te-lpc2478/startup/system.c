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

void delayMs(uint32_t delayInMs)
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

void Init_EMC(void) 
{
    volatile unsigned int i, dummy = dummy;

    // Initialize EMC and SDRAM for Samsung K4S561632H.
    SCS     |= 0x00000002;   // Reset EMC. 

    EMC_CTRL = 0x00000001; // Enable EMC and Disable Address mirror.

    PCONP   |= 0x00000800;   // Turn on EMC PCLK.

    PINSEL5  = 0x05050555; // CAS,RAS,CLKOUT0_1,DYCS0_1,CKEOUT0_1,DQMOUT0_1.
    PINSEL6  = 0x55555555; // D0-D15.
    PINSEL8  = 0x55555555; // A0-A15.
    PINSEL9  = 0x10555555; // A16-A23,OE,WE,BLS0_1,CS_0_.  

    EMC_DYN_RP     = 2; // Command period: 3(n+1) clock cycles.
    EMC_DYN_RAS    = 3; // RAS command period: 4(n+1) clock cycles.
    EMC_DYN_SREX   = 7; // Self-refresh period: 8(n+1) clock cycles.
    EMC_DYN_APR    = 2; // Data out to active: 3(n+1) clock cycles.
    EMC_DYN_DAL    = 5; // Data in to active: 5(n+1) clock cycles.
    EMC_DYN_WR     = 1;	// Write recovery: 2(n+1) clock cycles.
    EMC_DYN_RC     = 5;	// Active to Active cmd: 6(n+1) clock cycles.
    EMC_DYN_RFC    = 5;	// Auto-refresh: 6(n+1) clock cycles.
    EMC_DYN_XSR    = 7;	// Exit self-refresh: 8(n+1) clock cycles.
    EMC_DYN_RRD    = 1;	// Active bank A->B: 2(n+1) clock cycles.
    EMC_DYN_MRD    = 2;	// Load Mode to Active cmd: 3(n+1) clock cycles.

    EMC_DYN_RD_CFG  = 1; // Command delayed strategy.

    EMC_DYN_RASCAS0 = 0x00000303; //  Default setting, RAS latency 3 CCLKs, CAS latenty 3 CCLKs.

    EMC_DYN_CFG0 = 0x00000680;  // 256MB, 16Mx16, 4 banks, row=13, column=9.

    delayMs(100); // 100 ms.			

    EMC_DYN_CTRL = 0x00000183; // Mem clock enable, CLKOUT runs, send command: NOP.

    delayMs(200); // 200 ms.			

    EMC_DYN_CTRL = 0x00000103; // Send command: PRECHARGE-ALL, shortest possible refresh period.

    EMC_DYN_RFSH = 2; // Set 32 CCLKs between SDRAM refresh cycles.
    for(i = 0; i < 0x40; i++)
	NOP; // Wait 128 AHB clock cycles.
    EMC_DYN_RFSH = 28; // Set 28 x 16CCLKs=448CCLK=7us between SDRAM refresh cycles.

    //To set mode register in SDRAM, enter mode by issue
    //MODE command, after finishing, bailout and back to NORMAL mode.

    EMC_DYN_CTRL  = 0x00000083; // Mem clock enable, CLKOUT runs, send command: MODE.

    dummy = *((volatile unsigned int*)(SDRAM_BASE | (0x33 << 12))); // Set mode register in SDRAM.

    EMC_DYN_CTRL  = 0x00000000; // Send command: NORMAL.

    EMC_DYN_CFG0 |= 0x00080000; // Enable buffer.

    delayMs(1); // 1 ms.

    EMC_STA_WAITWEN0   = 0x2; // Selects the delay from chip select 0 to write enable.
    EMC_STA_WAITOEN0   = 0x2; // Selects the delay from chip select 0 or address change, whichever is later, to output enable.
    EMC_STA_WAITRD0    = 0x1f; // Selects the delay from chip select 0 to a read access.
    EMC_STA_WAITPAGE0  = 0x1f; // Selects the delay for asynchronous page mode sequential accesses for chip select 0.
    EMC_STA_WAITWR0    = 0x1f; // Selects the delay from chip select 0 to a write access.
    EMC_STA_WAITTURN0  = 0xf; // Selects the number of bus turnaround cycles for chip select 0.

    EMC_STA_CFG0       = 0x00000081; // Selects the memory configuration for static chip select 0.

    //EMCSTATICWAITWEN1   = 0x2; // Selects the delay from chip select 1 to write enable.
    //EMCSTATICWAITOEN1   = 0x2; // Selects the delay from chip select 1 or address change, whichever is later, to output enable.
    //EMCSTATICWAITRD1    = 0x8; // Selects the delay from chip select 1 to a read access.
    //EMCSTATICWAITPG1    = 0x1f; // Selects the delay for asynchronous page mode sequential accesses for chip select 1.
    //EMCSTATICWAITWR1    = 0x8; // Selects the delay from chip select 1 to a write access.
    //EMCSTATICWAITTURN1  = 0xf; // Selects the number of bus turnaround cycles for chip select 1.

    //EMCSTATICCNFG1      = 0x00000080; // Selects the memory configuration for static chip select 1.
}

void Init_PORT(void)
{
    // GPIO:
    // UART0:
    CONFIG_PINSEL(PINSEL0, 2, 1);//  PINSEL0_bit.P0_2 = 0x01;
    CONFIG_PINSEL(PINSEL0, 3, 1);//  PINSEL0_bit.P0_3 = 0x01;
}

void Init_LCD_controller(void)
{
    PCONP |= 0x00100000; // Power Control for CLCDC.
    //PINSEL11 = BIN8(1011); // TFT 16-bit (5:6:5 mode), LCD port is enabled.
    //PINSEL11 = BIN8(1101); // TFT 16-bit (1:5:5:5 mode), LCD port is enabled.
    //PINSEL11 = BIN8(1111); // TFT 24-bit (8:8:8 mode), LCD port is enabled.

    // Assign pin:
    PINSEL0 &= BIN32(11111111,11110000,00000000,11111111);
    PINSEL0 |= BIN32(00000000,00000101,01010101,00000000); // P0.4(LCD0), P0.5(LCD1), P0.6(LCD8), P0.7(LCD9), P0.8(LCD16), P0.9(LCD17). 
    PINMODE0&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE0|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up. 

    PINSEL3 &= BIN32(11110000,00000000,00000000,11111111);
    PINSEL3 |= BIN32(00000101,01010101,01010101,00000000); // P1.20(LCD10), P1.21(LCD11), P1.22(LCD12), P1.23(LCD13), P1.24(LCD14), P1.25(LCD15), P1.26(LCD20), P1.27(LCD21), P1.28(LCD22), P1.29(LCD23).
    PINMODE3&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE3|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL4 &= BIN32(11110000,00110000,00000000,00000011); // P2.0(LCDPWR) - not used
    PINSEL4 |= BIN32(00000101,01001111,11111111,11111100); //               P2.1(LCDLE), P2.2(LCDDCP), P2.3(LCDFP), P2.4(LCDENAB), P2.5(LCDLP), P2.6(LCD4), P2.7(LCD5), P2.8(LCD6), P2.9(LCD7), P2.12(LCD18), P2.13(LCD19). 
    PINMODE4&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE4|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL9 &= BIN32(11110000,11111111,11111111,11111111);
    PINSEL9 |= BIN32(00001010,00000000,00000000,00000000); // P4.28(LCD2), P4.29(LCD3).
    PINMODE9&= BIN32(00000000,00000000,00000000,00000000);
    //PINMODE9|= BIN32(00000000,00000000,00000000,00000000); // 00 Pull-up.

    PINSEL11&= BIN32(11111111,11111111,11111111,11110000);
    PINSEL11|= BIN32(00000000,00000000,00000000,00001101); // bit0=1 - LCD port is enabled.    bit1...3 = 110 TFT 16-bit. (1:5:5:5 mode) 
    //PINSEL11|= BIN32(00000000,00000000,00000000,00001011); // bit0=1 - LCD port is enabled.    bit1...3 = 101 TFT 16-bit. (5:6:5 mode)
    //PINSEL11|= BIN32(00000000,00000000,00000000,00001111); // bit0=1 - LCD port is enabled.    bit1...3 = 111 TFT 24-bit. (8:8:8 mode) 

    // SHUT, TPS61042.

    CONFIG_PINSEL(PINSEL7, 24, 0x00);//   PINSEL7_bit.P3_24 = 0x00;
    CONFIG_BIT(FIO3DIR, 24, 1);//  FIO3DIR_bit.P3_24 = 1;
    CONFIG_BIT(FIO3CLR, 24, 1);//  FIO3CLR_bit.P3_24 = 1;

    CONFIG_PINSEL(PINSEL1, 19, 0x00);//  PINSEL1_bit.P0_19 = 0x00;
    CONFIG_BIT(FIO0DIR, 19, 1);//  FIO0DIR_bit.P0_19  = 1;
    CONFIG_BIT(FIO0SET, 19, 1);//  FIO0SET_bit.P0_19  = 1;

    CONFIG_PINSEL(PINSEL4, 1, 0x00);//  PINSEL4_bit.P2_1  = 0x00;
    CONFIG_BIT(FIO2DIR, 1, 1);//  FIO2DIR_bit.P2_1  = 1;
    CONFIG_BIT(FIO2CLR, 1, 1);//  FIO2CLR_bit.P2_1  = 1;

    LCD_CFG    = 1;
    LCD_TIMH   = ((38<<24) | (20<<16) | (30<<8) | (((320/16)-1)<<2));
    LCD_TIMV   = ((15<<24) | (4<<16) | (3<<10) | 240);
    LCD_POL    = ((0<<27) | (0<<26) | (((320/1)-1)<<16) | (1<<14) | (1<<13)| (1<<12) | (1<<11)| (1<<6) | (0<<5) | (2<<0));

    LCD_CTRL   = ((0<<16) | (0<<10) | (0<<9) | (1<<8) | (1<<5) | (4<<1) | (0<<0)); // 100 = 16 bpp. 101 = 24 bpp (TFT panel only). 110 = 16 bpp, 5:6:5 mode. 

    LCD_UPBASE = LCD_BUFFER_ADDR;
    LCD_LPBASE = LCD_BUFFER_ADDR;

    int i;
    for(i=0; i < 50000; i++)
	NOP;

    LCD_CTRL |= 1;

    for(i=0; i < 50000; i++)
	NOP;

    LCD_CTRL |= 1<<11;
    PCONP |= 0x00100000; // Power Control for CLCDC.
    PINSEL11 = ( (5<<1) | 1);
}

void CLEAR_SDRAM(void)
{
    int i;
    uint32_t *Add_32 = (uint32_t *)(SDRAM_BASE);

    for(i = 0; i < SDRAM_SIZE; i++) {
	*Add_32++ = 0;
    }
}

void SET_SDRAM(void)
{
    int i;
    uint32_t *Add_32 = (uint32_t *)(SDRAM_BASE);

    for(i = 0; i < SDRAM_SIZE; i++) {
	*Add_32++ = 0xFFFFFFFF;
    }
}

void systemSetup(void)
{
    Init_MAP();

    Init_CPC();

    Init_MAM();

    Init_GPIO();

    Init_EMC();

    Init_PORT();

    Init_LCD_controller();

    Init_VIC();

    return;
}
