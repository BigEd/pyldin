#include <inttypes.h>
#include "config.h"
#include "irq.h"
#include "fio.h"
#include "kbd.h"
#include "kbdscans.h"

#define KBD_CLK_MASK (1 << 23)	// P2.23
#define KBD_CLK_FIO FIO2PIN
#define KBD_CLK_PORT 2

#define KBD_DAT_MASK (1 << 23)	// P3.23
#define KBD_DAT_FIO FIO3PIN
#define KBD_DAT_PORT 3

#define kbDAT (KBD_DAT_FIO & KBD_DAT_MASK)
#define kbCLK (KBD_CLK_FIO & KBD_CLK_MASK)
#define K_STARTBIT    1			
#define K_PARITYBIT  10			
#define K_STOPBIT    11			

#define KBUFFSIZE 16

void vGpio2Handler (void) __attribute__ ((interrupt("IRQ")));
static void decode(uint8_t key);
static void putkeybuf(uint32_t key);

static uint32_t keyBuf[KBUFFSIZE];	 //PS2 Keyboard buffer, the register to store characters key in
static uint8_t keyIn = 0;		 //Index into PS/2 key buf where next scan code will be inserted
static uint8_t keyOut = 0;		 //Index into PS/2 key buf where next scan code will be removed
static uint8_t keyRead = 0;		 //Number of keys read from the PS/2 keyboard

int keyboard_init(void)
{
    FIOInit(KBD_CLK_PORT, DIR_IN, KBD_CLK_MASK);
    FIOInit(KBD_DAT_PORT, DIR_IN, KBD_DAT_MASK);

    if ( Install_IRQ(EINT3_INT, (void *)vGpio2Handler, LOWEST_PRIORITY) == FALSE ) {
	return FALSE;
    }

    IO2_INT_EN_F |= KBD_CLK_MASK;

    return TRUE;
}

void vGpio2Handler(void)
{
    static uint8_t bitcount = 0;
    static uint8_t key_data;          // Holds the received scan code
    uint32_t clkstat;
    uint32_t datstat;
    uint32_t paritystat;
    if (IO_INT_STAT & (1 << 2)) {
	clkstat = kbCLK;             //check CLK pin state;
	datstat = kbDAT;             //check DAT pin state;

	bitcount++;

	if (bitcount == K_STARTBIT) {
	    if (datstat || clkstat)
		bitcount=0;
	    key_data=0;
	    paritystat=0;
	} else if (bitcount == K_PARITYBIT)
	    paritystat = datstat;
	else if (bitcount == K_STOPBIT) {
	    decode(key_data);
	    bitcount=0;
	} else {
	    // For all bits from 2, 3...9 (total 8-bit)
	    key_data= key_data >> 1;
	    if (datstat)
		key_data = key_data | 0x80;
	}
	IO2_INT_CLR = KBD_CLK_MASK;
    }
    EXTINT = 0x01;      //clear interrupt
    VICVectAddr = 0;
}

static uint16_t decode_key(uint32_t shift, uint8_t key)
{
    int i;
    if (!shift) {
	for (i = 0; unshifted[i][0] != key && unshifted[i][0]; i++);
	if (unshifted[i][0] == key)
	    return unshifted[i][1];
    } else {
	for (i = 0; shifted[i][0] != key && shifted[i][0]; i++);
	if (shifted[i][0] == key)
	    return shifted[i][1];
    }
    return 0;
}

static void decode(uint8_t scan)
{
    static uint32_t mods = 0;
    uint32_t oldmods = 0;
    uint32_t key = 0;

    if (!(mods & KEYB_KEYUP)) {
	switch (scan) {
	case BREAKCHAR:
	    mods |= KEYB_KEYUP ;
	    return;
	case NONUMCODE:
	case DOUBLECODE:
	    return;
	case LEFTSHIFT:
	case RIGHTSHIFT:
	    mods |= KEYB_SHIFT;
	    break;
	case CTRLKEY:
	    mods |= KEYB_CTRL;
	    break;
	case ALTKEY:
	    mods |= KEYB_ALT;
	    break;
	default:
	    key = decode_key(mods & KEYB_SHIFT, scan);
	}
	oldmods = mods;
    } else {
	oldmods = mods;
	switch (scan) {
	case LEFTSHIFT:
	case RIGHTSHIFT:
	    mods &= ~KEYB_SHIFT;
	    break;
	case CTRLKEY:
	    mods &= ~KEYB_CTRL;
	    break;
	case ALTKEY:
	    mods &= ~KEYB_ALT;
	    break;
	default:
	    key = decode_key(mods & KEYB_SHIFT, scan);
	}
	mods &= ~KEYB_KEYUP;
    }
    putkeybuf(key | oldmods);
}

static void putkeybuf(uint32_t key)
{
    if (keyRead < KBUFFSIZE) {
	keyRead++;
	keyBuf[keyIn++] = key;
	if (keyIn >= KBUFFSIZE)
	    keyIn = 0;
    }
}

unsigned int keyboard_get_key(void)
{
    unsigned int key=0;
    if (keyRead > 0) {
	keyRead--;
	key = keyBuf[keyOut];
	keyOut++;
	if (keyOut >= KBUFFSIZE) {
	    keyOut = 0;
	}
    }
    return key;
}
