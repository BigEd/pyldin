#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
//#include "config.h"
//#include "uart.h"
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

    return wrap_fs_open(r, name, flags, mode);
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

    return wrap_fs_write(r, fd, ptr, len);
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

    return wrap_fs_read(r, fd, ptr, len);
}

static long system_close_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    int fd = argv[1];

    if (fd == 0 || fd == 1 || fd == 2) {
	r->_errno = EBADF;
	return -1;
    }

    return wrap_fs_close(r, fd);
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

    return wrap_fs_lseek(r, fd, ptr, dir);
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

    return wrap_fs_fstat(r, fd, st);
}

static long system_isatty(uint32_t *argv)
{
    int fd = argv[0];
    if (fd == 0 || fd == 1 || fd == 2) {
	return 1;
    }

    return wrap_fs_isatty(fd);
}

static long system_exit(uint32_t *argv)
{
    asm("mov pc, #0");
    return -1;
}

static long system_mkdir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return wrap_fs_mkdir(r, (char *) argv[1], argv[2]);
}

static long system_rmdir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return wrap_fs_rmdir(r, (char *) argv[1]);
}

static long system_unlink_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return wrap_fs_unlink(r, (char *) argv[1]);
}

static long system_opendir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long)wrap_fs_opendir(r, (char *)argv[1]);
}

static long system_readdir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long)wrap_fs_readdir_r(r, (void *)argv[1], (struct dirent *)argv[2], (struct dirent **)argv[3]);
}

static long system_closedir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long)wrap_fs_closedir(r, (void *)argv[1]);
}

static long system_mountfs(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long)wrap_fs_mountfs(r, (char *)argv[1]);
}

static long system_umountfs(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long)wrap_fs_unmountfs(r, (char *)argv[1]);
}

void syscall_routine(unsigned long number, unsigned long *regs)
{
    char buf[128];

    if (number == AngelSWI) {
	sprintf(buf, "\r\n\r\n\r\n!!! Unknown Angel SWI %08X !!!\r\n\r\n\r\n", regs[0]);
	uart0Puts(buf);
    } else if (number == SystemSWI) {
	switch(regs[0]) {
	case SWI_WriteC:
	    uart0Putch(regs[1] & 0xff);
	    regs[0] = 0;
	    break;
	case SWI_Write:
	    uart0Puts((char *)regs[1]);
	    regs[0] = 0;
	    break;
	case SWI_WriteHex:
	    printf("0x%08X", regs[1]);
	    regs[0] = 0;
	    break;
	case SWI_ReadC:
	    regs[0] = uart0Getch();
	    break;
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
	case SWI_NEWLIB_Exit:
	    regs[0] = system_exit((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Mkdir_r:
	    regs[0] = system_mkdir_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Rmdir_r:
	    regs[0] = system_rmdir_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Opendir_r:
	    regs[0] = system_opendir_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Readdir_r:
	    regs[0] = system_readdir_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Closedir_r:
	    regs[0] = system_closedir_r((uint32_t *)regs[1]);
	    break;
	case SWI_MountFS:
	    regs[0] = system_mountfs((uint32_t *)regs[1]);
	    break;
	case SWI_UmountFS:
	    regs[0] = system_umountfs((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Unlink_r:
	    regs[0] = system_unlink_r((uint32_t *)regs[1]);
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
