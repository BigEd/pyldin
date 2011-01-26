#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"
#include "uart.h"
#include "swi.h"

#include "filesystem.c"

static long system_open_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    char *name = (char *)argv[1];
    int flags = argv[2];
    int mode = argv[3];

    if (!strcmp(name, "/dev/stdin"))
	return STDIN_FILENO;
    if (!strcmp(name, "/dev/stdout"))
	return STDOUT_FILENO;
    if (!strcmp(name, "/dev/stderr"))
	return STDERR_FILENO;

    return fs_open(r, name, flags, mode);
}

static long system_write_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];
    unsigned char *ptr = (unsigned char *)argv[2];
    int len = argv[3];

    if (fd == 0 || fd == 1 || fd == 2) {
	int i;
	for (i = 0; i < len; i++) {
	    if (*ptr == '\n')
		uart0Putch('\r');
	    uart0Putch(*ptr++);
	}
	return len;
    }

    return fs_write(r, fd, ptr, len);
}

static long system_read_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];
    unsigned char *ptr = (unsigned char *)argv[2];
    int len = argv[3];

    if (fd == 0 || fd == 1 || fd == 2) {
	int c;
	int i;
	for (i = 0; i < len; i++) {
	    while((c = uart0Getch()) < 0) ;

	    *ptr++ = c;
	    uart0Putch(c);

	    if (c == 0x0A) {
		uart0Putch(0x0D);
		return i + 1;
	    }
	}
	return i;
    }

    return fs_read(r, fd, ptr, len);
}

static long system_close_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];

    if (fd == 0 || fd == 1 || fd == 2) {
	r->_errno = EBADF;
	return -1;
    }

    return fs_close(r, fd);
}

static long system_lseek_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];
    int ptr = argv[2];
    int dir = argv[3];

    if (fd == 0 || fd == 1 || fd == 2) {
	r->_errno = EBADF;
	return -1;
    }

    return fs_lseek(r, fd, ptr, dir);
}

static long system_fstat_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];
    struct stat *st = (struct stat *) argv[2];

    if (fd == 0 || fd == 1 || fd == 2) {
	st->st_mode = S_IFCHR;
	return 0;
    }

    return fs_fstat(r, fd, st);
}

static long system_isatty(uint32_t *argv)
{
    int fd = argv[0];
    if (fd == 0 || fd == 1 || fd == 2) {
	return 1;
    }

    return fs_isatty(fd);
}

void syscall_routine(unsigned long number, unsigned long *regs)
{
    char buf[128];

    if (number == AngelSWI) {
	sprintf(buf, "\r\n\r\n\r\n!!! Unknown Angel SWI %08X !!!\r\n\r\n\r\n", regs[0]);
	uart0Puts(buf);
    } else if (number == SystemSWI) {
	switch(regs[0]) {
	case SWI_NEWLIB_Open_r:
	    regs[0] = system_open_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Read_r:
	    regs[0] = system_read_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Write_r:
	    regs[0] = system_write_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Close_r:
	    regs[0] = system_close_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Lseek_r:
	    regs[0] = system_lseek_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Fstat_r:
	    regs[0] = system_fstat_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Isatty:
	    regs[0] = system_isatty((uint32_t *)regs[1]);
	    break;
	default:
	    sprintf(buf, "\r\n\r\n\r\n!!! Unknown System SWI %08X !!!\r\n\r\n\r\n", regs[0]);
	    uart0Puts(buf);
	}
    } else {
	sprintf(buf, "\r\n\r\n\r\n!!! syscall %d (%08X %08X %08X %08X %08X) !!!\r\n\r\n\r\n", number, regs[0], regs[1], regs[2], regs[3], regs[4]);
	uart0Puts(buf);
    }
}
