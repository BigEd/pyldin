#include <stdio.h>
#include <string.h>
#include <time.h>

#define ENABLE_INT17_EMULATOR

#define O_INLINE static inline

#include "core/mc6800.h"
#include "core/swiemu.h"
#include "core/devices.h"
#include "core/mc6845.h"
#include "core/i8272.h"
#include "core/keyboard.h"
#include "core/printer.h"

#include "core/mc6800.c"
#include "core/devices.c"
//#include "core/floppy.c"
#include "core/keyboard.c"
#include "core/swiemu.c"
//#include "core/i8272.c"
#include "core/printer.c"
#include "core/mc6845.c"

void core_50Hz_irq(void)
{
    devices_set_tick50();
    mc6845_curBlink();
    mc6800_setIrq(1);
}
