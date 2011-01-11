#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include "core/mc6800.h"
#include "core/floppy.h"

int SWIemulator(int swi, byte *A, byte *B, word *X, byte *t, word *PC)
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
