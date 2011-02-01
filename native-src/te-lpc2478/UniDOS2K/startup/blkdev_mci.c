#include <inttypes.h>
#include <stdio.h>
#include "blkdev_mci.h"
#include "mci.h"
//#include <semaphore.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <fcntl.h>

struct _DEV_INFO {
    int		pDevice;	// File pointer to the device.
    void 	*AccessSem;
    long long	DiskSize;
    FF_T_UINT32	BlockSize;
};

BLK_DEV_MCI fnOpen(char *szDeviceName)
{
    BLK_DEV_MCI	ptDevInfo;
    int pDevice = 0;

//    pDevice = open(szDeviceName, O_RDWR);
    if (mmc_init()) {
	ptDevInfo = (struct _DEV_INFO *) malloc(sizeof(struct _DEV_INFO));
	if(ptDevInfo) {
	    MMCFG cfg;
	    if (!mmc_read_config (&cfg)) {
		mmc_fini();
		free(ptDevInfo);
		return NULL;
	    }
	    ptDevInfo->BlockSize = 512; /* cfg.read_blen */
	    ptDevInfo->pDevice = pDevice;
	    ptDevInfo->AccessSem = FF_CreateSemaphore();

	    return (BLK_DEV_MCI) ptDevInfo;
	}
    }
    return NULL;
}


void fnClose(BLK_DEV_MCI pDevice)
{
    mmc_fini();
    //close(pDevice->pDevice);
    FF_DestroySemaphore(pDevice->AccessSem);
    free(pDevice);
}

signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_MCI pDevice)
{
    unsigned short	total = 0;

    FF_PendSemaphore(pDevice->AccessSem);
    {
	for (total = 0; total < sectors; total++, buffer += pDevice->BlockSize) {
	    if (!mmc_read_sect(sector + total, buffer, 1))
		break;
	}
    }
    FF_ReleaseSemaphore(pDevice->AccessSem);

    return total;
}


signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_MCI pDevice)
{
    unsigned short	total = 0;

    FF_PendSemaphore(pDevice->AccessSem);
    {
	for (total = 0; total < sectors; total++, buffer += pDevice->BlockSize) {
	    if (!mmc_write_sect(sector + total, buffer, 1))
		break;
	}
    }
    FF_ReleaseSemaphore(pDevice->AccessSem);

    return total;
}

FF_T_UINT16 GetBlockSize(BLK_DEV_MCI pDevice)
{
    return pDevice->BlockSize;
}
