/*
 * hai_file.c
 *
 * Hardware Abstraction Interface implemented by file I/O under Linux.
 * implementation file.
 *
 * knightray@gmail.com
 * 10/27 2008
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "hai.h"
#include "pubstruct.h"

#include <inttypes.h>
#include "config.h"
#include "mci.h"

typedef struct _tdev{
	int32 total_sectors;
	int16 sector_size;
} tdev_t;

tdev_handle_t
HAI_initdevice(
	IN	byte * dev,
	IN	int16 sector_size)
{
	MMCFG cfg;
	tdev_t * pdev;

	if (!mmc_init())
	    return NULL;

	if (!mmc_read_config (&cfg)) {
	    mmc_fini();
	    return NULL;
	}

	pdev = (tdev_t *)malloc(sizeof(tdev_t));
	pdev->sector_size = 512; /* cfg.read_blen; */
	pdev->total_sectors = cfg.blocknr;

	return (tdev_handle_t)pdev;
}

int32
HAI_readsector(
	IN	tdev_handle_t hdev,
	IN	int32 addr,
	OUT	ubyte * ptr)
{
	tdev_t * pdev = (tdev_t *)hdev;

	if (!ptr || !pdev)
		return ERR_HAI_INVALID_PARAMETER;

	if (!mmc_read_sect(addr, ptr, 1))
		return ERR_HAI_READ;

	return HAI_OK;
}

int32
HAI_writesector(
	IN	tdev_handle_t hdev,
	IN	int32 addr,
	IN	ubyte * ptr)
{
	tdev_t * pdev = (tdev_t *)hdev;

	if (!ptr || !pdev)
		return ERR_HAI_INVALID_PARAMETER;
	
	if (!mmc_write_sect(addr, ptr, 1))
		return ERR_HAI_READ;

	return HAI_OK;
}

int32
HAI_closedevice(
	IN	tdev_handle_t hdev)
{
	tdev_t * pdev = (tdev_t *)hdev;

	if (!pdev)
		return ERR_HAI_INVALID_PARAMETER;
	

	mmc_fini();

	Free(pdev);

	return HAI_OK;
}

int32
HAI_getdevinfo(
	IN	tdev_handle_t hdev,
	OUT	tdev_info_t * devinfo)
{
	tdev_t * pdev = (tdev_t *)hdev;

	if (!devinfo || !pdev)
		return ERR_HAI_INVALID_PARAMETER;

	devinfo->free_sectors = 1000;
	return HAI_OK;
}

