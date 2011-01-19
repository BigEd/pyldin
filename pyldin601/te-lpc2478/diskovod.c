#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "mci.h"
#include "core/mc6800.h"
#include "core/floppy.h"

typedef struct {
    uint8_t	state;
    uint8_t	begin_head;
    uint16_t	begin_cylhead;
    uint8_t	type;
    uint8_t	end_head;
    uint16_t	end_cylhead;
    uint32_t	first_sector;
    uint32_t	total_sectors;
} __attribute__((packed)) mbr_entry;

static int mmc_inited = 0;

static struct {
    uint32_t start;
    uint32_t size;
    uint16_t tracks;
    uint16_t sectors;
    uint16_t heads;
    uint16_t inited;
} partition [2];

static int partition_init()
{
    char sector[512];

    if (!mmc_inited) {
	if (mmc_init()) {
	    if (mmc_read_sect(0, sector, 1)) {
		int i;
		for (i = 0; i < 2; i++) {
		    mbr_entry *mbr = (mbr_entry *) &sector[0x1be + 16 * i];
		    partition[i].inited = 0;
		    if (mbr->type == 0x01) {
			printf("FAT12 partition entry %d found!\r\n", i);
			partition[i].start = mbr->first_sector;
			partition[i].size = mbr->total_sectors;
			if (mmc_read_sect(partition[i].start, sector, 1)) {
			    if (sector[0x1fe] == 0x55 && sector[0x1ff] == 0xaa) {
				partition[i].size = sector[0x13] | (sector[0x14] << 8);
				partition[i].sectors = sector[0x18] | (sector[0x19] << 8);
				partition[i].heads = sector[0x1a] | (sector[0x1b] << 8);
				partition[i].tracks = partition[i].size / (partition[i].sectors * partition[i].heads);
				printf("FAT12 partition\r\nsize=%u\r\ntracks=%d\r\nsectors=%d\r\nheads=%d\r\n",
				    partition[i].size,
				    partition[i].tracks,
				    partition[i].sectors,
				    partition[i].heads);
				partition[i].inited = 1;
			    }
			}
		    }
		}
		mmc_inited = 150;
		return 1;
	    }
	}
    } else {
	mmc_inited = 150;
	return 1;
    }

    return 0;
}

void floppy_power_timeout(void)
{
    if (mmc_inited) {
	mmc_inited--;
	if (!mmc_inited)
	    mmc_fini();
    }
}

int floppy_init()
{
    return 0;
}

int floppy_readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst)
{
    char sector[512];
    if (partition_init()) {
	if (partition[Disk].inited) {
	    uint32_t n = partition[Disk].start + (Sector - 1) + Head * partition[Disk].sectors + Track * partition[Disk].sectors * partition[Disk].heads;
	    if (mmc_read_sect(n, sector, 1)) {
		memcpy(dst, sector, 512);
		return 0;
	    }
	}
    }

    return 0x40;
}

int floppy_writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src)
{
    char sector[512];
    if (partition_init()) {
	if (partition[Disk].inited) {
	    uint32_t n = partition[Disk].start + (Sector - 1) + Head * partition[Disk].sectors + Track * partition[Disk].sectors * partition[Disk].heads;
	    memcpy(sector, src, 512);
	    if (mmc_write_sect(n, sector, 1)) {
		return 0;
	    }
	}
    }

    return 0x40;
}

int floppy_formaTrack(int Disk, int Track, int Head)
{
    return 0x40;
}

void i8272_init(void)
{
}

void i8272_write(byte a, byte d)
{
}

byte i8272_read(byte a)
{
    return 0xff;
}
