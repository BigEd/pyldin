/*
 *  cdvd.h - LibCDVD
 *  Copyright (C) 2002, A.Lee & Nicholas Van Veen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdarg.h>
#include <string.h>

#include "cdvd_rpc.h"

int k_sceSifDmaStat(unsigned int id);
static unsigned sbuff[0x1300] __attribute__((aligned (64)));
static SifRpcClientData_t cd0 __attribute__((aligned(64)));

int cdvd_inited = 0;

int CDVD_Init()
{
    int ret;
    
    cd0.server = NULL;
    
    do 
    {
        if ((ret = SifBindRpc(&cd0, CDVD_IRX, 0)) < 0) 
            return -1;

        if (!cd0.server)
            nopdelay();
    }
    while(!cd0.server);
  
    cdvd_inited = 1;
    
    return 0;
}

int CDVD_DiskReady(int mode)
{
	if(!cdvd_inited) return -1;

	sbuff[0] = mode;

	SifCallRpc(&cd0,CDVD_DISKREADY,0,(void*)(&sbuff[0]),4,(void*)(&sbuff[0]),4,0,0);

	return sbuff[0];
}

int CDVD_FindFile(const char* fname, struct TocEntry* tocEntry)
{
	if(!cdvd_inited) return -1;

	strncpy((char*)&sbuff,fname,1024);

	SifCallRpc(&cd0,CDVD_FINDFILE,0,(void*)(&sbuff[0]),1024,(void*)(&sbuff[0]),sizeof(struct TocEntry)+1024,0,0);

	memcpy(tocEntry, &sbuff[256], sizeof(struct TocEntry));

	return sbuff[0];
}

void CDVD_Stop()
{
	if(!cdvd_inited) return;

	SifCallRpc(&cd0,CDVD_STOP,0,(void*)(&sbuff[0]),0,(void*)(&sbuff[0]),0,0,0);

	return;
}

int CDVD_TrayReq(int mode)
{
	if(!cdvd_inited) return -1;

	SifCallRpc(&cd0,CDVD_TRAYREQ,0,(void*)(&sbuff[0]),4,(void*)(&sbuff[0]),4,0,0);

	return sbuff[0];
}

int CDVD_GetDir(const char* pathname, const char* extensions, enum CDVD_getMode getMode, struct TocEntry tocEntry[], unsigned int req_entries, char* new_pathname)
{
	unsigned int num_entries;

	if(!cdvd_inited) return -1;

	// copy the requested pathname to the rpc buffer
	strncpy((char*)sbuff,pathname,1023);

	// copy in the extension list to the rpc buffer
	if (extensions == NULL)
	{
		// Can't copy in the extension list since there isnt one, so just null the string in the rpc buffer
		sbuff[1024/4] = 0;
	}
	else
	{
		strncpy((char*)&sbuff[1024/4],extensions,127);
	}

	sbuff[1152/4] = getMode;

	sbuff[1156/4] = (int)tocEntry;

	sbuff[1160/4] = req_entries;

	SifWriteBackDCache(tocEntry, req_entries*sizeof(struct TocEntry));

	// This will get the directory contents, and fill tocEntry via DMA
	SifCallRpc(&cd0,CDVD_GETDIR,0,(void*)(&sbuff[0]),1024+128+4+4+4,(void*)(&sbuff[0]),4+1024,0,0);

	num_entries = sbuff[0];

	if (new_pathname != NULL)
		strncpy(new_pathname,(char*)&sbuff[1],1023);

	return (num_entries);
}

void CDVD_FlushCache()
{
	if(!cdvd_inited) return;

	SifCallRpc(&cd0,CDVD_FLUSHCACHE,0,(void*)(&sbuff[0]),0,(void*)(&sbuff[0]),0,0,0);

	return;
}

unsigned int CDVD_GetSize()
{
	if(!cdvd_inited) return 0;

	SifCallRpc(&cd0,CDVD_GETSIZE,0,(void*)(&sbuff[0]),0,(void*)(&sbuff[0]),4,0,0);

	return sbuff[0];
}
