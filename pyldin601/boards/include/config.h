#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "lpc24xx.h"

#include "system.h"

#define HOST_BAUD_U0           (115200)
#define HOST_BAUD_U1           (115200)

#define PCLK Fpclk

#define BOARD_LED1_MASK (1 << 10)	// P1.10
#define BOARD_LED2_MASK (1 << 0)	// P2.0
#define BOARD_LED3_MASK (1 << 13)	// P1.13
#define BOARD_LED1_FIO FIO1PIN
#define BOARD_LED2_FIO FIO2PIN
#define BOARD_LED3_FIO FIO1PIN
#define BOARD_LED1_PORT 1
#define BOARD_LED2_PORT 2
#define BOARD_LED3_PORT 1

#define SDRAM_SIZE     2250000     // Samsung K4S561632HUC75, 256Mbit(4M x 16 bit x 4 Banks). 
#define SDRAM_BASE     0xA0000000  // 0xA000 0000 - 0xAFFF FFFF Dynamic memory bank 0.
#define SRAM_BASE      0x40000000  // 0x4000 0000 - 0x4000 FFFF On chip RAM (64 kB).

#define LCD_BUFFER_ADDR  SDRAM_BASE

#define NOP asm volatile (" nop ")

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif


#endif // _CONFIG_H_

