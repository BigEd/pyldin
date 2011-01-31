#ifndef __LPC24MCI_H__
#define __LPC24MCI_H__

/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
//#include "helper.h"
#include "../efsl/Inc/debug.h"
#include "../efsl/Inc/types.h"
#include "../efsl/Inc/config.h"
/*****************************************************************************/

struct hwInterface{
//	FILE 	*imageFile; 
	eint32 	sectorCount;
	euint32 readCount,writeCount;
};

typedef struct hwInterface hwInterface;

esint8 if_initInterface(hwInterface* file,eint8* fileName);
esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_finiInterface(hwInterface* file);

#endif
