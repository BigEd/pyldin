#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
//#include "config.h"
#include "uart.h"
#include "swi.h"

#ifdef USE_LIBFS_EFSL
#include "../efsl/filesystem.c"
#else
#include "filesystem.c"
#endif

static int empty_stdio(int c);

int (*stdio_func[3]) (int c) = { empty_stdio, empty_stdio, empty_stdio };

static int empty_stdio(int c)
{
    return 0;
    c = c;
}

static int system_stdio(int fd, int c)
{
    if (fd >= 0 && fd < 3) {
	return stdio_func[fd](c);
    }

    return c;
}

int redirect_stdio(int fd, void *func /*int (* func) (int c)*/)
{
    if (fd >= 0 && fd < 3) {
	stdio_func[fd] = func;
    }

    return 0;
}

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
		system_stdio(fd, '\r');
	    system_stdio(fd, *ptr++);
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
	    while((c = system_stdio(0, 0)) < 0) ;

	    if (c == 0x0D)
		c = 0x0A;

	    *ptr++ = c;
	    system_stdio(1, c);

	    if (c == 0x0A) {
		system_stdio(1, 0x0D);
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
    argv = argv;
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

static long system_rename_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return wrap_fs_rename(r, (char *) argv[1], (char *) argv[2]);
}

static long system_chdir_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return wrap_fs_chdir(r, (char *) argv[1]);
}

static long system_getcwd_r(uint32_t *argv)
{
    struct _reent *r = (struct _reent *) argv[0];
    return (long) wrap_fs_getcwd(r, (char *) argv[1], argv[2]);
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

void system_enable_irq(void);
asm("system_enable_irq:	\n\t"
    "push {r0}		\n\t"
    "mrs r0, CPSR	\n\t"
    "bic r0, r0, #0xc0	\n\t"
    "msr CPSR, R0	\n\t"
    "pop {r0}		\n\t"
    "bx  lr		\n\t"
    );

void syscall_routine(unsigned long number, unsigned long *regs)
{
    char buf[128];

    system_enable_irq();

    if (number == AngelSWI) {
	sprintf(buf, "\r\n\r\n\r\n!!! Unknown Angel SWI %08X !!!\r\n\r\n\r\n", (unsigned int)regs[0]);
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
	    printf("0x%08X", (unsigned int)regs[1]);
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
	case SWI_NEWLIB_Rename_r:
	    regs[0] = system_rename_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Chdir_r:
	    regs[0] = system_chdir_r((uint32_t *)regs[1]);
	    break;
	case SWI_NEWLIB_Getcwd_r:
	    regs[0] = system_getcwd_r((uint32_t *)regs[1]);
	    break;
	default:
	    sprintf(buf, "\r\n\r\n\r\n!!! Unknown System SWI %08X !!!\r\n\r\n\r\n", (unsigned int)regs[0]);
	    uart0Puts(buf);
	}
    } else {
	sprintf(buf, "\r\n\r\n\r\n!!! syscall %d (%08X %08X %08X %08X %08X) !!!\r\n\r\n\r\n", 
		(unsigned int)number, (unsigned int)regs[0], (unsigned int)regs[1], (unsigned int)regs[2], (unsigned int)regs[3], (unsigned int)regs[4]);
	uart0Puts(buf);
    }
}
