#ifndef FIO_H_
#define FIO_H_
/* bit 0 in SCS register, port 0/1 are regular ports when bit 0 
is 0,  fast ports when bit 0 is 1. */
#define GPIOM                          0x00000001

/* see master definition file lpc230x.h for more details */
#define REGULAR_PORT_DIR_BASE          GPIO_BASE_ADDR + 0x08
#define REGULAR_PORT_DIR_INDEX         0x10

#define HS_PORT_DIR_BASE               FIO_BASE_ADDR + 0x00
#define HS_PORT_DIR_INDEX              0x20

#define FAST_PORT          0x01
#define REGULAR_PORT       0x02

#define DIR_OUT            0x01
#define DIR_IN             0x02

extern void FIOInit( uint32_t PortNum, uint32_t PortDir, uint32_t Mask);

#endif /*FIO_H_*/
