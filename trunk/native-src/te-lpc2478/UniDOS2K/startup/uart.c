#include <inttypes.h>
#include "config.h"
#include "LPC_UTIL_DEFS.h"

#include "uart.h"

void uart0Init(uint16_t baud, uint8_t mode, uint8_t fmode)
{
  U0_TX_PINSEL_REG = ( U0_TX_PINSEL_REG & ~U0_TX_PINMASK ) | U0_TX_PINSEL;
  U0_RX_PINSEL_REG = ( U0_RX_PINSEL_REG & ~U0_RX_PINMASK ) | U0_RX_PINSEL;

  U0IER = 0x00;                         // disable all interrupts
  U0IIR;                                // clear interrupt ID
  U0RBR;                                // clear receive register
  U0LSR;                                // clear line status register

  // set the baudrate
  U0LCR = ULCR_DLAB_ENABLE;             // select divisor latches 
  U0DLL = (uint8_t)baud;                // set for baud low byte
  U0DLM = (uint8_t)(baud >> 8);         // set for baud high byte

  // set the number of characters and other
  // user specified operating parameters
  U0LCR = (mode & ~ULCR_DLAB_ENABLE);
  U0FCR = fmode;
}

int uart0Putch(int ch)
{
  while (!(U0LSR & ULSR_THRE))          // wait for TX buffer to empty
    continue;                           // also either WDOG() or swap()

  U0THR = (uint8_t)ch;

  return (uint8_t)ch;
}

const char *uart0Puts(const char *string)
{
  register char ch;

  while ((ch = *string) && (uart0Putch(ch) >= 0))
    string++;

  return string;
}

int uart0Getch(void)
{
  if (U0LSR & ULSR_RDR)                 // check if character is available
    return U0RBR;                       // return character

  return -1;
}
