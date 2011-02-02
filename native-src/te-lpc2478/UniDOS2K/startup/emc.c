#include <inttypes.h>
#include "config.h"
#include "emc.h"

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
