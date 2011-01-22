/*----------------------------------------------------------------------------
 *      R T L  -  F l a s h   F i l e   S y s t e m
 *----------------------------------------------------------------------------
 *      Name:    MCI_LPC23XX.C 
 *      Purpose: Multimedia Card Interface Driver for LPC23xx
 *      Rev.:    V3.21
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <inttypes.h>
#include "config.h"
#include "mcipriv.h"
#include "mci.h"

#include <string.h>

#define MCI_ENABLE_DMA

/* DMA Accessible memory buffer (USB memory) */
#define DMA_DATA_BUFFER 0x7FD00000

/* Wait timeouts, in multiples of 6 byte send over MCI (for 1 bit mode)      */
#define WR_TOUT           100000     /* ~ 200 ms with MCI clk 24MHz */
#define RD_STOP_TOUT      100        /* ~ 200 us with MCI clk 24MHz */

/* Wait time in for loop cycles */
#define DMA_TOUT          100000

/* Local variables */
static uint8_t  CardType;
static uint16_t CardRCA;
static uint32_t sernum;

/*----------------------------------------------------------------------------
 *      MMC Driver Functions
 *----------------------------------------------------------------------------
 *  Required functions for MMC driver module:
 *   - int mci_init ()
 *   - int mci_read_sect (uint32_t sect, uint8_t *buf, uint32_t cnt)
 *   - int mci_write_sect (uint32_t sect, uint8_t *buf, uint32_t cnt)
 *   - int mmc_read_config (MMCFG *cfg)
 *---------------------------------------------------------------------------*/

/* Local Function Prototypes */
static void mci_set_speed (uint32_t speed);
static void mci_bus_mode (uint32_t mode);
static int mci_send_acmd (void);
static int mci_set_address (void);
static int mci_read_cid (void);
static int mci_select_card (void);
static int mci_set_bus_4bit (void);
static int mci_set_block_len (void);
static int mci_cmd_read_block (uint32_t block, uint32_t cnt);
static int mci_cmd_write_block (uint32_t block, uint32_t cnt);
static uint32_t  mci_read_status (void);
static int mci_send_stop (void);
static int mci_wait_for_tran (void);
static uint32_t  mci_command (uint8_t cmd, uint32_t arg, uint32_t resp, uint32_t *rp);
#ifdef MCI_ENABLE_DMA
static void mci_dma_start (uint32_t mode, uint8_t *buf);
#endif

/*--------------------------- mci_init --------------------------------------*/

int mci_init (void) {
   /* Initialize and enable the Flash Card. */
   uint32_t i,rstat,rval[4];

   /* Power Up the MCI and DMA controller. */
#ifdef MCI_ENABLE_DMA
   PCONP |= 0x30000000;
#else
   PCONP |= 0x10000000;
#endif

   /* MCIPWR pin active low */
   SCS &= ~8;

   /* Enable MCI Pins on P1 */
   PINSEL2 &= 0xFC3F030F; //BIN32(11111100,00111111,00000011,00001111); // P1.2    P1.3    P1.5    P1.6     P1.7     P1.11    P1.12
   PINSEL2 |= 0x0280A8A0; //BIN32(00000010,10000000,10101000,10100000); // MCICLK, MCICMD, MCIPWR, MCIDAT0, MCIDAT1, MCIDAT2, MCIDAT3

   /* Clear all pending interrupts. */
   MCI_COMMAND   = 0;
   MCI_DATA_CTRL = 0;
   MCI_CLEAR     = 0x7FF;

   /* Power up, switch on VCC for the Flash Card. */
   MCI_POWER  = 0x02;
   for (i = 0; i < 50000; i++);

   mci_set_speed (LOW_SPEED);

   /* Power on the Flash Card. */
   MCI_POWER |= 0x01;
   for (i = 0; i < 50000; i++);

   /* Reset the card, send CMD0. */
   mci_command (GO_IDLE_STATE, 0, RESP_NONE, NULL);

   /* Set Open Drain output control for MMC */
   mci_bus_mode (OPEN_DRAIN_MODE);

   CardType = CARD_NONE;
   /* First try MMC, send CMD1. */
   for (i = 0; i < 100; i++) {
      rstat = mci_command (SEND_OP_COND, OCR_INDEX, RESP_SHORT, &rval[0]);
      if (!(rstat & MCI_CMD_TIMEOUT) && rval[0] & 0x80000000) {
         CardType = CARD_MMC;
         break;
      }
   }

   if (CardType == CARD_NONE) {
      /* Check for SD card, clear Open Drain output control. */
      mci_bus_mode (PUSH_PULL_MODE);
      for (i = 0; i < 500; i++) {
         if (mci_send_acmd () == TRUE) {
            rstat = mci_command (SEND_APP_OP_COND, 
                                 OCR_INDEX, RESP_SHORT, &rval[0]);
            if (!(rstat & MCI_CMD_TIMEOUT) && rval[0] & 0x80000000) {
               /* OK, SD card initialized. */
               CardType = CARD_SD;
               break;
            }
         }
      }
   }
   if (CardType == CARD_NONE) {
      /* Failed, no card found. */
      goto fail;
   }

   /* Initialize the Card to SD/MMC mode. */
   if (mci_read_cid () == FALSE) {
      goto fail;
   }
   if (mci_set_address () == FALSE) {
      goto fail;
   }

   /* Disable Open Drain mode for MMC. */
   if (CardType == CARD_MMC) {
      mci_bus_mode (PUSH_PULL_MODE);
   }

   /* Data Transfer Mode, end of Card-Identification Mode. */
   mci_set_speed (HIGH_SPEED);

   if (mci_select_card () == FALSE) {
      goto fail;
   }

   if (CardType == CARD_SD) {
      /* Use wide 4-bit bus for SD */
      MCI_CLOCK |= 0x0800;
      for (i = 0; i < 100; i++);
      if (mci_set_bus_4bit () == FALSE) {
         /* Failed to enable 4-bit bus. */
         goto fail;
      }
   }

   /* Set block length to 512 bytes. */
   if (mci_set_block_len () == FALSE) {
fail:
      MCI_POWER = 0x00;
      return (FALSE);
   }

   /* Success, card initialized. */
   return (TRUE);
}

/*--------------------------- mci_fini --------------------------------------*/

void mmc_fini (void)
{
   MCI_POWER = 0x00;
}

/*--------------------------- mci_set_speed ---------------------------------*/

static void mci_set_speed (uint32_t speed) {
   /* Set a MCI clock speed to desired value. */
   uint32_t i,clkdiv;

   if (speed == HIGH_SPEED) {
      /* Max. 25 MBit used for Data Transfer. */
      clkdiv = 2;
   } else {
      /* Max. 400 kBit used in Card Initialization. */
      clkdiv = 120;
   }
   MCI_CLOCK = (MCI_CLOCK & ~0xFF) | 0x300 | clkdiv;

   /* delay 3MCLK + 2PCLK before next write */
   for ( i = 0; i < 100; i++ );
}


/*--------------------------- mci_bus_mode ----------------------------------*/

static void mci_bus_mode (uint32_t mode) {
   /* Set MCI Bus mode to Open Drain or Push Pull. */
   uint32_t i;

   if (mode == OPEN_DRAIN_MODE) {
      MCI_POWER |= 0x40;
   }
   else {
      MCI_POWER &= ~0x40;
   }
   /* A small delay after switching mode. */
   for (i = 0; i < 100; i++);
}


/*--------------------------- mci_send_acmd ---------------------------------*/

static int mci_send_acmd (void) {
   /* Send CMD55 to enable ACMD */
   uint32_t arg,rstat,rval;

   arg = 0;
   if (CardType == CARD_SD) {
      /* Use address from SET_RELATIVE_ADDR. */
      arg = CardRCA << 16;
   }
   rstat = mci_command (APP_CMD, arg, RESP_SHORT, &rval);
   if (rstat == 0 && (rval & STAT_ACMD_ENABLE)) {
      return (TRUE);
   }
   return (FALSE);
}


/*--------------------------- mci_set_address -------------------------------*/

static int mci_set_address (void) {
   /* Set Relative Address, send CMD3 after CMD2. */
   uint32_t i,arg,rstat,rval;

   arg = 0;
   if (CardType == CARD_MMC) {
      /* Fix the RCA address for MMC card. */
      arg = 0x00010000;
   }

   for (i = 0; i < 20; i++) {
      rstat = mci_command (SET_RELATIVE_ADDR, arg, RESP_SHORT, &rval);
      if (!(rstat & MCI_CMD_TIMEOUT) && (rval & 0x0F00) == 0x0500) {
         /* Response is back and correct. */
         CardRCA = rval >> 16;
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_read_cid ----------------------------------*/

static int mci_read_cid (void) {
   /* Check CID, send CMD2 after CMD1 (MMC) or ACMD41 (SD). */
   uint32_t i,rstat,rval[4];

   for (i = 0; i < 20; i++) {
      rstat = mci_command (ALL_SEND_CID, 0, RESP_LONG, &rval[0]);
      if (!(rstat & MCI_CMD_TIMEOUT)) {
         /* Response is back and correct. */
         if (CardType == CARD_SD) {
            /* Serial Number for SD Card. */
            sernum = (rval[2] << 8)  | (rval[3] >> 24);
         }
         else {
            /* Serial Number for MMC Card. */
            sernum = (rval[2] << 16) | (rval[3] >> 16);
         }
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_select_card -------------------------------*/

static int mci_select_card (void) {
   /* Select the Card, send CMD7 after CMD9, inter-change state */
   /* between STBY and TRANS after this command. */
   uint32_t i,arg,rstat,rval;

   arg = 0x00010000;
   if (CardType == CARD_SD) {
      /* Use address from SET_RELATIVE_ADDR. */
      arg = CardRCA << 16;
   }

   for (i = 0; i < 200; i++) {
      rstat = mci_command (SELECT_CARD, arg, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0F00) == 0x0700) {
         /* Should be in STBY state now and ready. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_set_bus_4bit ------------------------------*/

static int mci_set_bus_4bit (void) {
   /* Select 4-bit bus width for SD Card. */
   uint32_t i,rstat,rval;

   for (i = 0; i < 20; i++) {
      if (mci_send_acmd () == FALSE) {
         continue;
      }
      /* Send ACMD6 command to set the bus width. */
      rstat = mci_command (SET_ACMD_BUS_WIDTH, BUS_WIDTH_4BITS, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0F00) == 0x0900) {
         /* Response is back and correct. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_set_block_len -----------------------------*/

static int mci_set_block_len (void) {
   /* Set block length to 512 bytes. */
   uint32_t i,rstat,rval;

   for (i = 0; i < 20; i++) {
      /* Send ACMD6 command to set the bus width. */
      rstat = mci_command (SET_BLOCK_LEN, 512, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0F00) == 0x0900) {
         /* Response is back and correct. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_cmd_read_block ----------------------------*/

static int mci_cmd_read_block (uint32_t block, uint32_t cnt) {
   /* Send a command to Read Single/Multiple blocks. */
   uint32_t i,rstat,rval;
   uint8_t  cmd;

   cmd = READ_BLOCK;
   if (cnt > 1) {
      cmd = READ_MULT_BLOCK;
   }
   block *= 512;
   for (i = 0; i < 20; i++) {
      rstat = mci_command (cmd, block, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0F00) == 0x0900) {
         /* Ready and in TRAN state. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_cmd_write_block ---------------------------*/

static int mci_cmd_write_block (uint32_t block, uint32_t cnt) {
   /* Send a command to Write Single/Multiple block. */
   uint32_t i,rstat,rval;
   uint8_t  cmd;

   cmd = WRITE_BLOCK;
   if (cnt > 1) {
      cmd = WRITE_MULT_BLOCK;
   }
   block *= 512;
   for (i = 0; i < 20; i++) {
      rstat = mci_command (cmd, block, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0F00) == 0x0900) {
         /* Ready and in TRAN state. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_read_status -------------------------------*/

static uint32_t mci_read_status (void) {
   /* Read the status of Flash Card. */
   uint32_t i,arg,rstat,rval;

   arg = 0x00010000;
   if (CardType == CARD_SD) {
      /* Use address from SET_RELATIVE_ADDR. */
      arg = CardRCA << 16;
   }

   for (i = 0; i < 200; i++) {
      rstat = mci_command (SEND_STATUS, arg, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0100)) {
         /* The Ready bit should be set, state TRAN or RCV. */
         return (rval);
      }
   }
   return (MCI_RESP_INVALID);
}


/*--------------------------- mci_send_stop ---------------------------------*/

static int mci_send_stop (void) {
   /* Stop transmission, Flash Card is in wrong state. */
   uint32_t i,rstat,rval;

   for (i = 0; i < 20; i++) {
      rstat = mci_command (STOP_TRANSMISSION, 0, RESP_SHORT, &rval);
      if (rstat == 0 && (rval & 0x0100)) {
         /* The Ready bit should be set. */
         return (TRUE);
      }
   }
   return (FALSE);
}


/*--------------------------- mci_wait_for_tran -----------------------------*/

static int mci_wait_for_tran (void) {
   /* Wait for Card state TRAN. */
   uint32_t i;

   for (i = WR_TOUT; i; i--) {
      /* Wait for Card state TRAN to continue. */
      if ((mci_read_status () & 0x0F00) == 0x0900) {
         break;
      }
   }
   if (i == 0) {
      /* Previous request has Failed. */
      mci_send_stop ();
      return (FALSE);
   }
   return (TRUE);
}


/*--------------------------- mmc_command -----------------------------------*/

static uint32_t mci_command (uint8_t cmd, uint32_t arg, uint32_t resp_type, uint32_t *rp) {
   /* Send a Command to Flash card and get a Response. */
   uint32_t cmdval,stat;

   cmd   &= 0x3F;
   cmdval = 0x400 | cmd;
   switch (resp_type) {
      case RESP_SHORT:
         cmdval |= 0x40;
         break;
      case RESP_LONG:
         cmdval |= 0xC0;
         break;
   }
   /* Send the command. */
   MCI_ARGUMENT = arg;
   MCI_COMMAND  = cmdval;

   if (resp_type == RESP_NONE) {
      /* Wait until command finished. */
      while (MCI_STATUS & MCI_CMD_ACTIVE);
      MCI_CLEAR = 0x7FF;
      return (0);
   }

   for (;;) {
      stat = MCI_STATUS;
      if (stat & MCI_CMD_TIMEOUT) {
         MCI_CLEAR = stat & MCI_CLEAR_MASK;
         return (stat);
      }
      if (stat & MCI_CMD_CRC_FAIL) {
         MCI_CLEAR = stat & MCI_CLEAR_MASK;
         if ((cmd == SEND_OP_COND)      ||
             (cmd == SEND_APP_OP_COND)  ||
             (cmd == STOP_TRANSMISSION)) {
            MCI_COMMAND = 0;
            break;
         }
         return (stat);
      }
      if (stat & MCI_CMD_RESP_END) {
         MCI_CLEAR = stat & MCI_CLEAR_MASK;
         break;
      }
   }
   if ((MCI_RESP_CMD & 0x3F) != cmd) {
      if ((cmd != SEND_OP_COND)     &&
          (cmd != SEND_APP_OP_COND) &&
          (cmd != ALL_SEND_CID)     &&
          (cmd != SEND_CSD))         {
         return (MCI_RESP_INVALID);
      }
   }
   if (rp == NULL) {
      /* Response pointer undefined. */
      return (MCI_RESP_INVALID);
   }
   /* Read MCI response registers */
   rp[0] = MCI_RESP0;
   if (resp_type == RESP_LONG) {
      rp[1] = MCI_RESP1;
      rp[2] = MCI_RESP2;
      rp[3] = MCI_RESP3;
   }
   return (0);
}


#ifdef MCI_ENABLE_DMA
/*--------------------------- mci_dma_start ---------------------------------*/

static void mci_dma_start (uint32_t mode, uint8_t *buf) {
   /* Configure DMA controller Ch0 for read or write. */

   if (mode == DMA_READ) {
      /* Transfer from MCI-FIFO to memory. */
      GPDMA_CH0_SRC  = (uint32_t)&MCI_FIFO;
      GPDMA_CH0_DEST = (uint32_t)buf;
      GPDMA_CH0_LLI  = 0;
      /* The burst size set to 8, transfer size 512 bytes. */
      GPDMA_CH0_CTRL = (512 >> 2)   | (0x02 << 12) | (0x02 << 15) | 
                       (0x02 << 18) | (0x02 << 21) | (1 << 27)    | (1u << 31);
      GPDMA_CH0_CFG  = 0x10001 | (0x04 << 1) | (0x00 << 6) | (0x06 << 11);
   }
   else {
      /* Transfer from memory to MCI-FIFO. */
      GPDMA_CH0_SRC  = (uint32_t)buf;
      GPDMA_CH0_DEST = (uint32_t)&MCI_FIFO;
      GPDMA_CH0_LLI  = 0;
      /* The burst size set to 8, transfer size 512 bytes. */
      GPDMA_CH0_CTRL = (512 >> 2)   | (0x02 << 12) | (0x02 << 15) |
                       (0x02 << 18) | (0x02 << 21) | (1 << 26)    | (1u << 31);
      GPDMA_CH0_CFG  = 0x10001 | (0x00 << 1) | (0x04 << 6) | (0x05 << 11);
   }
   /* Enable DMA channels, little endian */
   GPDMA_INT_TCCLR = 0x01;
   GPDMA_CONFIG    = 0x01;
}
#endif

/*--------------------------- mci_read_sect ---------------------------------*/

int mci_read_sect (uint32_t sect, uint8_t *buf, uint32_t cnt) {
   /* Read one or more 512 byte sectors from Flash Card. */

   if (mci_wait_for_tran () == FALSE) {
      /* Card not in TRAN state. */
      return (FALSE);
   }

   if (mci_cmd_read_block (sect, cnt) == FALSE) {
      /* Command Failed. */
      return (FALSE);
   }

   /* Set MCI Transfer registers. */
   MCI_DATA_TMR  = DATA_RD_TOUT_VALUE;
   MCI_DATA_LEN  = cnt * 512;

#ifdef MCI_ENABLE_DMA
   uint32_t i;

   /* Start DMA Peripheral to Memory transfer. */
   mci_dma_start (DMA_READ, (uint8_t *)DMA_DATA_BUFFER);
   MCI_DATA_CTRL = 0x9B;

   for (i = DMA_TOUT; i; i--) {
      if (GPDMA_RAW_INT_TCSTAT & 0x01) {
         /* Data transfer finished. */
         break;
      }
   }

   if (i == 0) {
      /* DMA Transfer timeout. */
      return (FALSE);
   }

   memcpy(buf, (void *)DMA_DATA_BUFFER, 512);
#else
    uint32_t *ptr = (uint32_t *) buf;
    uint32_t total = 0;
    uint32_t status = 0;

    MCI_DATA_CTRL = 0x93;

    while(1) {
	if (MCI_STATUS & MCI_RX_HALF_FULL) {
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    *ptr++ = MCI_FIFO;
	    total += 32;
	}
	if (total == cnt * 512 || (status = MCI_STATUS) & (MCI_DATA_TIMEOUT | MCI_DATA_END | MCI_DATA_BLK_END | MCI_DATA_CRC_FAIL))
	    break;
    }

   if (total != cnt * 512 || status & MCI_DATA_CRC_FAIL)
      return (FALSE);
#endif

   if (cnt > 1) {
      /* Stop reading Multiple sectors. */
      mci_send_stop ();
   }
   return (TRUE);
}


/*--------------------------- mci_write_sect --------------------------------*/

int mci_write_sect (uint32_t sect, uint8_t *buf, uint32_t cnt) {
   /* Write a 512 byte sector to Flash Card. */

   if (mci_wait_for_tran () == FALSE) {
      /* Card not in TRAN state. */
      return (FALSE);
   }

   if (mci_cmd_write_block (sect, cnt) == FALSE) {
      /* Command Failed. */
      return (FALSE);
   }

#ifdef MCI_ENABLE_DMA
   uint32_t i,j;

   for (j = 0; j < cnt; buf += 512, j++) {
      /* Set MCI Transfer registers. */
      MCI_DATA_TMR  = DATA_WR_TOUT_VALUE;
      MCI_DATA_LEN  = 512;

      memcpy((void *)DMA_DATA_BUFFER, buf, 512);
      /* Start DMA Memory to Peripheral transfer. */
      mci_dma_start (DMA_WRITE, (void *)DMA_DATA_BUFFER);
      MCI_DATA_CTRL = 0x99;

      for (i = DMA_TOUT; i; i--) {
         if (GPDMA_RAW_INT_TCSTAT & 0x01) {
            /* Data transfer finished. */
            break;
         }
      }

      if (i == 0) {
         /* DMA Data Transfer timeout. */
         mci_send_stop ();
         /* Write request Failed. */
         return (FALSE);
      }

      if (cnt == 1) {
         return (TRUE);
      }

      /* Wait until Data Block sent to Card. */
      while (MCI_STATUS != (MCI_DATA_END | MCI_DATA_BLK_END)) {
         if (MCI_STATUS & (MCI_DATA_CRC_FAIL | MCI_DATA_TIMEOUT)) {
            /* If error while Data Block sending occured. */
            mci_send_stop ();
            /* Write request Failed. */
            return (FALSE);
         }
      }
      for (i = WR_TOUT; i; i--) {
         if ((mci_read_status () & 0x0F00) == 0x0D00) {
            /* Buffer available for further sending, card state RCV. */
            break;
         }
      }
   }
#else
    int i;
    uint32_t *ptr = (uint32_t *) buf;
    uint32_t total = 0;
    uint32_t status = 0;

    MCI_DATA_TMR  = DATA_WR_TOUT_VALUE;
    MCI_DATA_LEN  = cnt * 512;

    MCI_DATA_CTRL = 0x91;

    while(1) {
	if (MCI_STATUS & MCI_TX_HALF_EMPTY) {
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    MCI_FIFO = *ptr++;
	    total += 32;
	}
	if (total == cnt * 512 || (status = MCI_STATUS) & (MCI_DATA_TIMEOUT | MCI_DATA_END | MCI_DATA_BLK_END | MCI_DATA_CRC_FAIL))
	    break;
    }

   if (total != cnt * 512 || status & MCI_DATA_CRC_FAIL)
      return (FALSE);

   /* Wait until Data Block sent to Card. */
   while (MCI_STATUS != (MCI_DATA_END | MCI_DATA_BLK_END)) {
      if (MCI_STATUS & (MCI_DATA_CRC_FAIL | MCI_DATA_TIMEOUT)) {
         /* If error while Data Block sending occured. */
         mci_send_stop ();
         /* Write request Failed. */
         return (FALSE);
      }
   }
   for (i = WR_TOUT; i; i--) {
      if ((mci_read_status () & 0x0F00) == 0x0D00) {
         /* Buffer available for further sending, card state RCV. */
         break;
      }
   }
#endif
   mci_send_stop ();

   /* Write request Ok. */
   return (TRUE);
}


/*--------------------------- mci_read_config -------------------------------*/

int mci_read_config (MMCFG *cfg) {
   /* Read MMC/SD Card device configuration. */
   uint32_t i,rstat,arg,v,m,rval[4];

   /* Wait if potential Write in progress. */
   mci_wait_for_tran ();

   /* Deselect the Card, transit to STBY state. */
   mci_command (SELECT_CARD, 0, RESP_NONE, NULL);

   /* Read the CID - Card Identification. */
   cfg->sernum = sernum;

   /* Read the CSD - Card Specific Data. */
   arg = 0x00010000;
   if (CardType == CARD_SD) {
      /* Use address from SET_RELATIVE_ADDR. */
      arg = CardRCA << 16;
   }

   for (i = 20; i; i--) {
      rstat = mci_command (SEND_CSD, arg, RESP_LONG, &rval[0]);
      if (rstat == 0) {
         /* Response is back and correct. */
         break;
      }
   }
   if (i == 0) {
      /* Read CSD failed. */
      return (FALSE);
   }

   /* Read Block length. */
   v = (rval[1] >> 16) & 0x0F;
   cfg->read_blen = 1 << v;

   /* Write Block length */
   v = (rval[3] >> 22) & 0x0F;
   cfg->write_blen = 1 << v;

   /* Total Number of blocks */
   v = ((rval[1] << 2) | (rval[2] >> 30)) & 0x0FFF;
   m =  (rval[2] >> 15) & 0x07;
   cfg->blocknr = (v + 1) << (m + 2);

   /* Re-select the Card, back to TRAN state. */
   return (mci_select_card ());
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
