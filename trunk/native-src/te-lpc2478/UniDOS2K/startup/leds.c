#include <inttypes.h>
#include "config.h"
#include "fio.h"

void Init_Leds(void)
{
    FIOInit(BOARD_LED1_PORT, DIR_OUT, BOARD_LED1_MASK);
    FIOInit(BOARD_LED2_PORT, DIR_OUT, BOARD_LED2_MASK);
    FIOInit(BOARD_LED3_PORT, DIR_OUT, BOARD_LED3_MASK);
}
