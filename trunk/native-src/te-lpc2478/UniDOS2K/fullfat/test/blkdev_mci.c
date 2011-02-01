#define _LARGEFILE64_SOURCE

#include "blkdev_mci.h"
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

struct _DEV_INFO {
    int			pDevice;	// File pointer to the device.
    void 		*AccessSem;
    long long	DiskSize;
    FF_T_UINT32	BlockSize;
};

BLK_DEV_MCI fnOpen(char *szDeviceName)
{
    BLK_DEV_MCI	ptDevInfo;
    int pDevice;

    pDevice = open(szDeviceName, O_RDWR);
    if (pDevice != -1) {
	ptDevInfo = (struct _DEV_INFO *) malloc(sizeof(struct _DEV_INFO));
	if(ptDevInfo) {
	    ptDevInfo->BlockSize = 512;
	    ptDevInfo->pDevice = pDevice;
	    ptDevInfo->AccessSem = FF_CreateSemaphore();

	    return (BLK_DEV_MCI) ptDevInfo;
	}
    }
    return NULL;
}


void fnClose(BLK_DEV_MCI pDevice)
{
    close(pDevice->pDevice);
    FF_DestroySemaphore(pDevice->AccessSem);
    free(pDevice);
}

signed int fnRead(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_MCI pDevice)
{
    unsigned long long address;
    unsigned long	Read;

    address = (unsigned long long) sector * pDevice->BlockSize;

    FF_PendSemaphore(pDevice->AccessSem);
    {
	lseek64(pDevice->pDevice, address, SEEK_SET);
	Read = read(pDevice->pDevice, buffer, pDevice->BlockSize * sectors);
    }
    FF_ReleaseSemaphore(pDevice->AccessSem);

    return Read / pDevice->BlockSize;
}


signed int fnWrite(unsigned char *buffer, unsigned long sector, unsigned short sectors, BLK_DEV_MCI pDevice)
{
    unsigned long long address;
    unsigned long Written;

    address = (unsigned long long) sector * pDevice->BlockSize;

    FF_PendSemaphore(pDevice->AccessSem);
    {
	lseek64(pDevice->pDevice, address, SEEK_SET);
	Written = write(pDevice->pDevice, buffer, pDevice->BlockSize * sectors);
    }
    FF_ReleaseSemaphore(pDevice->AccessSem);

    return Written / pDevice->BlockSize;
}

FF_T_UINT16 GetBlockSize(BLK_DEV_MCI pDevice)
{
    return pDevice->BlockSize;
}
