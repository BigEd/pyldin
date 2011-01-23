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
#include <errno.h>
#include "uart.h"

#include "syscallsfs.c"

int _open_r(struct _reent *r, const char *pathname, int flags, int mode)
{
    printf("open(%s, %08X, %08x)\n", pathname, flags, mode);

    if (!strcmp(pathname, "/dev/stdin"))
	return STDIN_FILENO;
    if (!strcmp(pathname, "/dev/stdout"))
	return STDOUT_FILENO;
    if (!strcmp(pathname, "/dev/stderr"))
	return STDERR_FILENO;

    return fs_open(r, pathname, flags, mode);
}

// new code for _read_r provided by Alexey Shusharin - Thanks
_ssize_t _read_r(struct _reent *r, int file, void *ptr, size_t len)
{
    if (file == STDIN_FILENO) {
	int c;
	unsigned int  i;
	unsigned char *p = (unsigned char*)ptr;

	for (i = 0; i < len; i++) {
	    while((c = uart0Getch()) < 0) ;

	    *p++ = c;
	    uart0Putch(c);

	    if (c == 0x0A) {
		uart0Putch(0x0D);
		return i + 1;
	    }
	}

	return i;
    }

    if (file != STDOUT_FILENO && file != STDERR_FILENO)
	return fs_read(r, file, ptr, len);

    r->_errno = EBADF;
    return -1;
}

_ssize_t _write_r (struct _reent *r, int file, const void *ptr, size_t len)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
	size_t i;
	const unsigned char *p;

	p = (const unsigned char*) ptr;

	for (i = 0; i < len; i++) {
	    if (*p == '\n' )
		uart0Putch('\r');
	    uart0Putch(*p++);
	}

	return len;
    }

    if (file != STDIN_FILENO)
	return fs_write(r, file, ptr, len);

    r->_errno = EBADF;
    return -1;
}

int _close_r(struct _reent *r, int file)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO) {
	r->_errno = EBADF;
	return -1;
    }

    return fs_close(r, file);
}

_off_t _lseek_r(struct _reent *r, int file, _off_t ptr, int dir)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO) {
	r->_errno = EBADF;
	return -1;
    }

    return fs_lseek(r, file, ptr, dir);
}


int _fstat_r(
    struct _reent *r, 
    int file, 
    struct stat *st)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO) {
	st->st_mode = S_IFCHR;
	return 0;
    }

    r->_errno = EBADF;
    return -1;
}

int _isatty(int file); /* avoid warning */

int _isatty(int file)
{
	return 1;
}

#if 0
static void _exit (int n) {
label:  goto label; /* endless loop */
}
#endif 

/* "malloc clue function" */

	/**** Locally used variables. ****/
extern char end[];              /*  end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr;		/* Points to current end of the heap.	*/

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

void * _sbrk_r(
    struct _reent *_s_r, 
    ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error	*/

	if (!heap_ptr) {	/*  Initialize if first time through.		*/
		heap_ptr = end;
	}
	base = heap_ptr;	/*  Point to end of heap.			*/
	heap_ptr += nbytes;	/*  Increase heap.				*/
	
	return base;		/*  Return pointer to start of new heap area.	*/
}
