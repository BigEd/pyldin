#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "console/screen.h"
#include "console/console.h"
#include "kbd.h"
#include "uart.h"

int unidos(void);

int init_syscalls(void);
int redirect_stdio(int fd, void * /*int (* func) (int c)*/);

int main(void)
{
    init_syscalls();

    uart0Init(UART_BAUD(HOST_BAUD_U0), UART_8N1, UART_FIFO_8); // setup the UART

    redirect_stdio(0, uart0Getch);
    redirect_stdio(1, uart0Putch);
    redirect_stdio(2, uart0Putch);

#ifdef USE_LCD
    screen_init();
    console_init();

    redirect_stdio(1, console_putchar);
    redirect_stdio(2, console_putchar);
#endif
#ifdef USE_KBD
    keyboard_init();
    redirect_stdio(0, console_getchar);
#endif

    printf("UniBIOS Version build (%s)\n", __DATE__);

    unidos();

    return 0;
}
