#include "config.h"
#include <inttypes.h>
#include "irq.h"
#include "fio.h"

void FIOInit( uint32_t PortNum, uint32_t PortDir, uint32_t Mask )
{
    if ( PortDir == DIR_OUT )
	(*(volatile unsigned long *)(HS_PORT_DIR_BASE + PortNum * HS_PORT_DIR_INDEX)) |= Mask;
    else
	(*(volatile unsigned long *)(HS_PORT_DIR_BASE + PortNum * HS_PORT_DIR_INDEX)) &= ~Mask;
    return;
}
