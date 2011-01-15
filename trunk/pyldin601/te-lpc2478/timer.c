#include <sys/time.h>
#include <inttypes.h>
#include "timer.h"
#include "config.h"
#include "irq.h"

int vTimer0Init (unsigned long ulDelay)
{
    if ( Install_IRQ(TIMER0_INT, (void *)vTimer0Handler, LOWEST_PRIORITY) == FALSE ) {
	return (FALSE);
    }
    T0TCR = 0x02;			/* reset timer */
    T0PR = 0x00;			/* set prescaler to zero */
    T0MR0 = ulDelay * (Fpclk / 1000);	/* TxMR - comparing register */
    T0IR = 0xff;			/* reset all interrrupts */
    T0MCR = 0x03;			/* interrupt timer on match */
    T0TCR = 0x01;			/* start timer */

    return (TRUE);
}

void vTimer0Handler (void)
{
    TimerHandler();
    /* Reset the interrupt source */
    T0IR = 1;
    VICVectAddr = 0;
}
