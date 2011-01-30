/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : lpc2000_spi.c                                                     *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card connected to an LPC2xxx.        *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2005 Martin Thomas    *
\*****************************************************************************/

/*****************************************************************************/
#include "interfaces/LPC2000_regs.h"
#include "interfaces/lpc2000_spi.h"
#include "interfaces/sd.h"
#include "config.h"
#include "utils.h"
#include "atmel_flash.h"
//#include "fwu_utils.h"
/*****************************************************************************/
#define EFS_START 0x20000



esint8 if_initInterface(hwInterface* file, eint8* opts)
{

  SPI_Init();
  ATF_Init();
  file->sectorCount = 200;
  return 0;
}
/*****************************************************************************/ 

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
      //s_memcpy(buf,(void *)(address*512)+EFS_START,512);
      ATF_PageRead(buf,address*2,0,256);
      ATF_PageRead(buf+256,address*2+1,0,256);
      return 0;
//	return(sd_readSector(file,address,buf,512));
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
	esint8 retval;

// flash_erase_sectors((address*512)+EFS_START,512,60000);
 //return flash_write((address*512)+EFS_START,(U32)buf,60000,512);
  retval= ATF_PageWrite(buf,CHIP_BUFFER_1,address*2,0,256);
  if(retval==0)
  retval= ATF_PageWrite(buf+256,CHIP_BUFFER_1,address*2+1,0,256);
  
 // if(retval==0)
 // retval=1;
  return retval;
 //return 0;
//return(sd_writeSector(file,address, buf));
}
/*****************************************************************************/ 

esint8 if_setPos(hwInterface* file,euint32 address)
{
	return(0);
}
/*****************************************************************************/ 

/*****************************************************************************/ 

void if_spiInit(hwInterface *iface)
{
  SPI_Init();
  ATF_Init();
}
/*****************************************************************************/
