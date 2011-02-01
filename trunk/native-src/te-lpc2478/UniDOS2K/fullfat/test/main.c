#include <stdio.h>
#include "fullfat.h"
#include "blkdev_mci.h"

#define PARTITION_NUMBER	0					// FullFAT can mount primary partitions only. Specified at Runtime.


static void FF_PrintDir(FF_DIRENT *pDirent) {
    unsigned char attr[5] = { '-','-','-','-', '\0' };	// String of Attribute Flags.
    if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
	attr[0] = 'R';
    if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
	attr[1] = 'H';
    if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
	attr[2] = 'S';
    if(pDirent->Attrib & FF_FAT_ATTR_DIR)
	attr[3] = 'D';
#ifdef FF_TIME_SUPPORT
    printf("%02d.%02d.%02d  %02d:%02d  %s  %12lu  %s\n", pDirent->CreateTime.Day, pDirent->CreateTime.Month, pDirent->CreateTime.Year, pDirent->CreateTime.Hour, pDirent->CreateTime.Minute, attr, pDirent->Filesize, pDirent->FileName);
#else
    printf(" %s %12lu    %s\n", attr, pDirent->Filesize, pDirent->FileName);
#endif
}

void ls_cmd(FF_IOMAN *pIoman)
{
    FF_DIRENT mydir;					// DIRENT object.
    FF_ERROR tester = 0;

    tester = FF_FindFirst(pIoman, &mydir, "/");

    while (tester == 0) {
	FF_PrintDir(&mydir);
	tester = FF_FindNext(pIoman, &mydir);
    }

}



int main(int argc, char *argv[])
{
    BLK_DEV_MCI		hDisk;
    FF_IOMAN		*pIoman;
    FF_ERROR		Error = FF_ERR_NONE;

    hDisk = fnOpen(argv[1]);
    if (hDisk) {
	pIoman = FF_CreateIOMAN(NULL, 4096, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)
	    if (pIoman) {
		//---------- Register a Block Device with FullFAT.
		Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
		if (Error) {
		    printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
		}
		//---------- Try to Mount the Partition with FullFAT.
		Error = FF_MountPartition(pIoman, PARTITION_NUMBER);
		if (Error) {
		    if(hDisk) {
			fnClose(hDisk);
		    }
		    FF_DestroyIOMAN(pIoman);
		    printf("FullFAT Couldn't mount the specified parition!\n");
		    printf("FF_MountPartition() function returned with Error %ld\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
		    return -1;
		}

		printf("Mounted!!!\n");
		ls_cmd(pIoman);

		FF_UnmountPartition(pIoman);						// Dis-mount the mounted partition from FullFAT.
		FF_DestroyIOMAN(pIoman);							// Clean-up the FF_IOMAN Object.
	    }
	fnClose(hDisk);
    } else
	printf("Can't open!\n");

    return 0;
}
