#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "mci.h"
#include "core/mc6800.h"
#include "core/floppy.h"

//#define DEBUG

//#define xprintf printf
#define xprintf(...) {}

typedef struct {
    uint8_t	state;
    uint8_t	begin_head;
    uint16_t	begin_cylhead;
    uint8_t	type;
    uint8_t	end_head;
    uint16_t	end_cylhead;
    uint32_t	first_sector;
    uint32_t	total_sectors;
} __attribute__((packed)) mbr_partition_entry;

static int mmc_inited = 0;

static struct {
    uint32_t start;
    uint32_t size;
    uint16_t tracks;
    uint16_t sectors;
    uint16_t heads;
    uint16_t inited;
} partition [4];

void led_control(int led, int v);

static int partition_init()
{
    byte sector[512];
    mbr_partition_entry mbr_part[4];

    if (!mmc_inited) {
	if (mmc_init()) {
	    if (mmc_read_sect(0, sector, 1)) {
		int i, j;
		memcpy(&mbr_part[0], &sector[0x1be], sizeof(mbr_partition_entry) * 4);
		for (i = 0, j = 0; i < 4; i++) {
		    partition[j].inited = 0;
		    if (mbr_part[i].type == 0x01) {
			xprintf("FAT12 partition entry %d found!\n", i);
			partition[j].start = mbr_part[i].first_sector;
			partition[j].size = mbr_part[i].total_sectors;
			if (mmc_read_sect(partition[i].start, sector, 1)) {
			    if (((sector[0x1fe] == 0x55 && sector[0x1ff] == 0xaa)) ||
				((sector[0x1fe] == 0xa5 && sector[0x1ff] == 0x5a))) {
				partition[j].size = sector[0x13] | (sector[0x14] << 8);
				partition[j].sectors = sector[0x18] | (sector[0x19] << 8);
				partition[j].heads = sector[0x1a] | (sector[0x1b] << 8);
				partition[j].tracks = partition[i].size / (partition[i].sectors * partition[i].heads);
				xprintf("FAT12 partition\nsize=%u\r\ntracks=%d\r\nsectors=%d\r\nheads=%d\r\n",
				    partition[j].size,
				    partition[j].tracks,
				    partition[j].sectors,
				    partition[j].heads);
				partition[j].inited = 1;
				j++;
			    }
			}
		    } else
			xprintf("Unknown partition entry %d type %02X\n", i, mbr_part[i].type);
		}
		xprintf("Total %d usable partition(s) found.\n\n", j);
		for (; j < 4; j++)
		    partition[j].inited = 0;
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

int floppy_status(int Disk)
{
    int ret = 0xc0;
    led_control(Disk, 1);
    if (partition_init()) {
	if (partition[Disk].inited)
	    ret = 0;
    }

    led_control(Disk, 0);
    return ret;
}

int floppy_readSector(int Disk, int Track, int Sector, int Head, unsigned char *dst)
{
    int ret = 0x40;
    led_control(Disk, 1);
#ifdef DEBUG
    printf("Read Sector disk=%d track=%d sector=%d head=%d\r\n", Disk, Track, Sector, Head);
#endif
    if (partition_init()) {
	if (partition[Disk].inited) {
	    uint32_t n = partition[Disk].start + (Sector - 1) + Head * partition[Disk].sectors + Track * partition[Disk].sectors * partition[Disk].heads;
	    if (mmc_read_sect(n, dst, 1))
		ret = 0;
	}
    }

    led_control(Disk, 0);
    return ret;
}

int floppy_writeSector(int Disk, int Track, int Sector, int Head, unsigned char *src)
{
    int ret = 0x40;
    led_control(Disk, 1);
#ifdef DEBUG
    printf("Write Sector disk=%d track=%d sector=%d head=%d\r\n", Disk, Track, Sector, Head);
#endif
    if (partition_init()) {
	if (partition[Disk].inited) {
	    uint32_t n = partition[Disk].start + (Sector - 1) + Head * partition[Disk].sectors + Track * partition[Disk].sectors * partition[Disk].heads;
	    if (mmc_write_sect(n, src, 1))
		ret = 0;
	}
    }

    led_control(Disk, 0);
    return ret;
}

int floppy_formaTrack(int Disk, int Track, int Head)
{
    int ret = 0x40;
    led_control(Disk, 1);
    Track = Track;
    Head = Head;
    led_control(Disk, 0);
    return ret;
}

#ifdef ENABLE_INT17_EMULATOR
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
#endif
