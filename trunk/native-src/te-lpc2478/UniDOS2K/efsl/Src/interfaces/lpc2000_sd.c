#include "interfaces/sd.h"
#include "lpc_2148.h"
#include "type.h"
#include "ssp.h"

#define SD_CARD BIT20
#define SD_SELECTION_DIR_SET (FIO0DIR |= SD_CARD)
#define SD_SELECTION_SET (FIO0SET = SD_CARD)
#define SD_SELECTION_CLR (FIO0CLR = SD_CARD)

esint8 if_initInterface(hwInterface* file, eint8* opts)
{
  if_spiInit(file);
  if(sd_Init(file)<0)
  {
    //		DBG((TXT("Card failed to init, breaking up...\n")));
    return(-1);
  }
  if(sd_State(file)<0)
  {
    //		DBG((TXT("Card didn't return the ready state, breaking up...\n")));
    return(-2);
  }
  file->sectorCount=4; /* FIXME ASAP!! */
  //DBG((TXT("Init done...\n")));
  return(0);
}

euint8 if_spiSend(hwInterface *iface, euint8 outgoing)
{
  euint8 incoming=0;
  
  while ( !(SSPSR & SSPSR_TNF) );
  SSPDR = outgoing;
  while (!(SSPSR & SSPSR_RNE));
  incoming = SSPDR;
  
  return(incoming);
}

void if_spiInit(hwInterface *iface)
{
  spi1_init();
  SD_SELECTION_DIR_SET;
  SD_SELECTION_CLR;	
}

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
  return(sd_readSector(file,address,buf,512));
}

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
  return(sd_writeSector(file,address, buf));
}



