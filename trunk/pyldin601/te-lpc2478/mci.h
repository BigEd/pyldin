#ifndef _MCI_H_
#define _MCI_H_

/* Card Type definitions */
#define CARD_NONE       0
#define CARD_MMC        1
#define CARD_SD         2

typedef struct mmcfg {
   uint32_t sernum;
   uint32_t blocknr;
   uint16_t read_blen;
   uint16_t write_blen;
} MMCFG;

int mmc_init (void);
int mmc_write_sect (uint32_t sect, uint8_t *buf, uint32_t cnt);
int mmc_read_sect (uint32_t sect, uint8_t *buf, uint32_t cnt);
int mmc_read_config (MMCFG *cfg);
int mmc_fini (void);

#endif
