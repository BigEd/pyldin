#include <stdio.h>

#include "core/mc6800.h"
#include "core/floppy.h"

#ifdef ENABLE_INT17_EMULATOR
void INT17emulator(byte *A, byte *B, word *X, byte *t, word *PC)
{
    byte sect_buf[512];
    word bukva, i2;

    int devs  = mc6800_memr(*X) & 0x01;
    int track = mc6800_memr(*X + 1);
    int head  = mc6800_memr(*X + 2) & 0x01;
    int sect  = mc6800_memr(*X + 3);
    int offs  = (mc6800_memr(*X + 4) << 8) | mc6800_memr(*X + 5);

    bukva = mc6800_memr(0xed20) << 8; 
    bukva |= mc6800_memr(0xed21);

//    if (m601a == 0)
	bukva += 81;
//    else 
//	bukva += 159;

    switch(*A) {
	case 0x80:
	case 0:
	    *A = floppy_init();
	    break;

	case 1:
	    mc6800_memw(bukva, 0x52);
	    *A = floppy_readSector(devs, track, sect, head, sect_buf);
	    for (i2 = 0; i2 < 512; i2++)
		mc6800_memw(offs + i2, sect_buf[i2]);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 2:	
	    mc6800_memw(bukva, 0x57);
	    for (i2 = 0; i2 < 512; i2++)
		sect_buf[i2] = mc6800_memr(offs + i2);
	    *A = floppy_writeSector(devs, track, sect, head, sect_buf);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 3:	
	    mc6800_memw(bukva, 0x53);
	    *A = floppy_readSector(devs, track, sect, head, sect_buf);
	    mc6800_memw(bukva, 0x20);
	    break;

	case 4:	
	    mc6800_memw(bukva, 0x46);
	    *A = floppy_formaTrack(devs, track, head);
	    mc6800_memw(bukva, 0x20);
	    break;

	default: 
	    *A = 0;
	    break;
    }
}
#endif

O_INLINE int SWIemulator(int swi, byte *A, byte *B, word *X, byte *t, word *PC)
{
#ifdef ENABLE_INT17_EMULATOR
    switch(swi) {
    case 0x17:
	INT17emulator(A, B, X, t, PC);
	return 1;
    }
#endif
    return 0;
}
