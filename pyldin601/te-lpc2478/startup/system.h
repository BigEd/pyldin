#ifndef _TARGET_H_
#define _TARGET_H_

#ifdef __cplusplus
   extern "C" {
#endif

#define PLL_MValue     11 // Fin = 12MHz Fcco = 288 MHz M = (288*(10^6)*1)/(2*12*10^6) N = 1 M = 12.
#define PLL_NValue     0
#define CCLKDivValue   3  // Fcco/4 = 72 MHz.
#define USBCLKDivValue 5  // Fcco/6 = 48 MHz.

//#define Fosc	12000000
#define Fcclk	72000000
//#define Fcco	288000000
#define Fpclk	(Fcclk / 1)

#define USE_USB 1

//This segment should not be modified
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LongToBin(n) (((n >> 21) & 0x80) | \
                      ((n >> 18) & 0x40) | \
                      ((n >> 15) & 0x20) | \
                      ((n >> 12) & 0x10) | \
                      ((n >>  9) & 0x08) | \
                      ((n >>  6) & 0x04) | \
                      ((n >>  3) & 0x02) | \
                      ((n      ) & 0x01))

#define __BIN(n) LongToBin(0x##n##l)

#define BIN8(n)                       __BIN(n)
#define BIN(n)                        __BIN(n)
#define BIN16(b1,b2)        ((        __BIN(b1)  <<  8UL) + \
                                      __BIN(b2))
#define BIN32(b1,b2,b3,b4) ((((uint32_t)__BIN(b1)) << 24UL) + \
                            (((uint32_t)__BIN(b2)) << 16UL) + \
                            (((uint32_t)__BIN(b3)) <<  8UL) + \
                              (uint32_t)__BIN(b4))

#define CONFIG_PINSEL(a, p, v) { \
    unsigned long t = a; \
    unsigned long p1 = (p & 0x0f) << 1; \
    t &= ~(0x03 << p1); \
    t |= (v << p1); \
    a = t; \
}

#define CONFIG_PINMODE CONFIG_PINSEL

#define CONFIG_BIT(a, p, v) { \
    unsigned long t = a; \
    t &= ~(1 << p); \
    t |= (v << p); \
    a = t; \
}

extern void systemSetup(void);

#ifdef __cplusplus
   }
#endif
 
#endif /* end _TARGET_H_ */
