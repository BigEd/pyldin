/*****************************************************************************/ 
#include <inttypes.h>
#include "config.h"
#include "mci.h"
#include "lpc24mci.h"
/*****************************************************************************/ 

/* ****************************************************************************  
 * short if_initInterface(hwInterface* file, char* fileName)
 * Description: This function should bring the hardware described in file in a
 * ready state to receive and retrieve data.
 * Return value: Return 0 on succes and -1 on failure.
*/
esint8 if_initInterface(hwInterface* file, eint8* fileName)
{
	MMCFG cfg;

	if (!mmc_init())
	    return (-1);

	if (!mmc_read_config (&cfg)) {
	    mmc_fini();
	    return (-1);
	}

	file->sectorCount = cfg.blocknr;
	file->readCount=file->writeCount=0;

	return(0);
}
/*****************************************************************************/ 

/* ****************************************************************************  
 * esint8 if_finiInterface(hwInterface* file)
 * Description: This function shutdown storage device hardware
 *
 * Return value: Return 0 on succes and -1 on failure.
*/
esint8 if_finiInterface(hwInterface* file)
{
    mmc_fini();

    return 0;
}
/*****************************************************************************/ 

/* ****************************************************************************  
 * short if_readBuf(hwInterface* file,unsigned long address,unsigned char* buf)
 * Description: This function should fill the characterpointer buf with 512 
 * bytes, offset by address*512 bytes. Adress is thus a LBA address.
 * Return value: Return 0 on success and -1 on failure.
*/
esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
	/*printf("READ  %li\n",address);*/
	if (!mmc_read_sect(address, buf, 1))
		return (-1);
	file->readCount++;
	return(0);
}
/*****************************************************************************/ 

/* ****************************************************************************  
 * short if_writeBuf(hwInterface* file,unsigned long address,unsigned char* buf)
 * Description: This function writes 512 bytes from uchar* buf to the hardware
 * disc described in file. The write offset should be address sectors of 512 bytes.
 * Return value: Return 0 on success and -1 on failure.
*/
esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
	/*printf("WRITE %li\n",address);*/
	if (!mmc_write_sect(address, buf, 1))
		return (-1);
	file->writeCount++;
	return(0);
}
/*****************************************************************************/ 
