/***********************************************************************/
/*                                                                     */
/*  SYSCALLS.C:  System Calls Remapping                                */
/*  most of this is from newlib-lpc and a Keil-demo                    */
/*                                                                     */
/*  these are "reentrant functions" as needed by                       */
/*  the WinARM-newlib-config, see newlib-manual                        */
/*  collected and modified by Martin Thomas                            */
/*  TODO: some more work has to be done on this                        */
/***********************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <reent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#include "swi.h"

static inline int do_SystemSWI (int reason, void *arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; swi %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (reason), "r" (arg), "i" (SystemSWI) /* Inputs */
       : "r0", "r1", "lr"
		/* Clobbers r0 and r1, and lr if in supervisor mode */);
  return value;
}

int _open_r(struct _reent *r, const char *pathname, int flags, int mode)
{
    int volatile block[4];

    block[0] = (int) r;
    block[1] = (int) pathname;
    block[2] = flags;
    block[3] = mode;
    return do_SystemSWI(SWI_NEWLIB_Open_r, (void *)block);
}

_ssize_t _read_r(struct _reent *r, int file, void *ptr, size_t len)
{
    int volatile block[4];

    block[0] = (int) r;
    block[1] = file;
    block[2] = (int) ptr;
    block[3] = len;
    return do_SystemSWI(SWI_NEWLIB_Read_r, (void *)block);
}

_ssize_t _write_r (struct _reent *r, int file, const void *ptr, size_t len)
{
    int volatile block[4];

    block[0] = (int) r;
    block[1] = file;
    block[2] = (int) ptr;
    block[3] = len;
    return do_SystemSWI(SWI_NEWLIB_Write_r, (void *)block);
}

int _close_r(struct _reent *r, int file)
{
    int volatile block[2];

    block[0] = (int) r;
    block[1] = file;
    return do_SystemSWI(SWI_NEWLIB_Close_r, (void *)block);
}

_off_t _lseek_r(struct _reent *r, int file, _off_t ptr, int dir)
{
    int volatile block[4];

    block[0] = (int) r;
    block[1] = file;
    block[2] = ptr;
    block[3] = dir;
    return do_SystemSWI(SWI_NEWLIB_Lseek_r, (void *)block);
}

int _fstat_r(struct _reent *r, int file, struct stat *st)
{
    int volatile block[3];

    block[0] = (int) r;
    block[1] = file;
    block[2] = (int) st;
    return do_SystemSWI(SWI_NEWLIB_Fstat_r, (void *)block);
}

#if 1
int _isatty(int file)
{
    int volatile block[1];

    block[0] = file;
    return do_SystemSWI(SWI_NEWLIB_Isatty, (void *)block);
}

int _gettimeofday_r(struct _reent *r, struct timeval *tp, void *tzp)
{
    int volatile block[3];

    block[0] = (int) r;
    block[1] = (int) tp;
    block[2] = (int) tzp;
    return do_SystemSWI(SWI_NEWLIB_Gettimeofday_r, (void *)block);
}

void _exit(int n)
{
    int volatile block[1];

    block[0] = n;
    do_SystemSWI(SWI_NEWLIB_Exit, (void *)block);
}
#endif

/* "malloc clue function" */

/**** Locally used variables. ****/
extern char *__stack_end__; 	/* Defined by startup                   */

extern char __end__[];          /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr;		/* Points to current end of the heap.	*/

#define STACK_BUFFER 16384

/************************** _sbrk_r *************************************/
/*  Support function.  Adjusts end of heap to provide more memory to	*/
/* memory allocator. Simple and dumb with no sanity checks.		*/
/*  struct _reent *r	-- re-entrancy structure, used by newlib to 	*/
/*			support multiple threads of operation.		*/
/*  ptrdiff_t nbytes	-- number of bytes to add.			*/
/*  Returns pointer to start of new heap area.				*/
/*  Note:  This implementation is not thread safe (despite taking a	*/
/* _reent structure as a parameter).  					*/
/*  Since _s_r is not used in the current implementation, the following	*/
/* messages must be suppressed.						*/

void * _sbrk_r(struct _reent *r, ptrdiff_t nbytes)
{
    char  *base;		/*  errno should be set to  ENOMEM on error	*/

    if (!heap_ptr) {	/*  Initialize if first time through.		*/
	heap_ptr = __end__;
	printf("heap_ptr = %p\n", __end__);
	printf("stack_ptr = %p\n", __stack_end__);
    }

    if ((__stack_end__ - (heap_ptr + nbytes)) > STACK_BUFFER) {
	base = heap_ptr;	/*  Point to end of heap.			*/
	heap_ptr += nbytes;	/*  Increase heap.				*/
	return base;		/*  Return pointer to start of new heap area.	*/
    }

    r->_errno = ENOMEM;
    return (void *) -1;
}

#if 1
void __libc_fini_array()
{
}
#endif
