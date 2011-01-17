#ifndef _TIMER_H_
#define _TIMER_H__

#define CLOCK_CONF_SECOND 1000

int timer_init(unsigned long ulDelay);

extern void timer_handler(void);

#endif
