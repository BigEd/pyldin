#ifndef _SYSCALLSFS_C_
#define _SYSCALLSFS_C_

#include <stdio.h>
#include <sys/fcntl.h>
#include "tffs.h"

#define MAX_FILEDESCR	32

extern tffs_handle_t htffs_mmc;

#define htffs htffs_mmc

typedef struct {
    tfile_handle_t hfile;
} filedesc;

static filedesc fd_list[MAX_FILEDESCR];

static int inited = 0;

static void fs_check_is_inited(void)
{
    int i;
    if (!inited) {
	// reserved fds
	fd_list[0].hfile = (tfile_handle_t ) 0xffffffff; // stdin
	fd_list[1].hfile = (tfile_handle_t ) 0xffffffff; // stdout
	fd_list[2].hfile = (tfile_handle_t ) 0xffffffff; // stderr
	for (i = 3; i < MAX_FILEDESCR; i++)
	    fd_list[i].hfile = NULL;
	inited = 1;
    }
}

static int fs_get_fd(void)
{
    int i;
    for (i = 0; i < MAX_FILEDESCR; i++) {
	if (fd_list[i].hfile == NULL)
	    return i;
    }
    return -1;
}

static int fs_open(struct _reent *r, const char *pathname, int flags, int mode)
{
    fs_check_is_inited();

    int fd = fs_get_fd();
    if (fd >= 0) {
	int ret;
	char *m;
	switch (flags & (O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_TRUNC)) {
	case (O_RDONLY):
	    m = "r";
	    break;
	case (O_WRONLY):
	case (O_WRONLY | O_TRUNC):
	    m = "w";
	    break;
	case (O_WRONLY | O_APPEND):
	    m = "a";
	    break;
	default:
	    fprintf(stderr, "open() unsupported flags %08x\n", flags);
	    r->_errno = EACCES;
	    return -1;
	}
	if ((ret = TFFS_fopen(htffs, (byte *) pathname, m, &fd_list[fd].hfile)) != TFFS_OK) {
		fprintf(stderr, "TFFS_fopen %d\n", ret);
		fd_list[fd].hfile = NULL;
		r->_errno = EACCES;
		return -1;
	}
	return fd;
    }

    r->_errno = ENFILE;
    return -1;
}

static _ssize_t fs_read(struct _reent *r, int file, void *ptr, size_t len)
{
    if (fd_list[file].hfile) {
	int ret;
	if ((ret = TFFS_fread(fd_list[file].hfile, len, (unsigned char *)ptr)) < 0) {
	    if (ret != ERR_TFFS_FILE_EOF) {
		fprintf(stderr, "TFFS_fread %d\n", ret);
		r->_errno = EIO;
		return -1;
	    }
	}
	return ret;
    }
    r->_errno = EBADF;
    return -1;
}

static _ssize_t fs_write(struct _reent *r, int file, const void *ptr, size_t len)
{
    if (fd_list[file].hfile) {
	int ret;
	if ((ret = TFFS_fwrite(fd_list[file].hfile, len, (unsigned char *)ptr)) < 0) {
	    fprintf(stderr, "TFFS_fwrite %d\n", ret);
	    r->_errno = EIO;
	    return -1;
	}
	return ret;
    }
    r->_errno = EBADF;
    return -1;
}

static int fs_close(struct _reent *r, int file)
{
    if (fd_list[file].hfile) {
	int ret;
	if ((ret = TFFS_fclose(fd_list[file].hfile)) != TFFS_OK) {
	    fprintf(stderr, "TFFS_fclose %d\n", ret);
	    r->_errno = EIO;
	    return -1;
	}
	fd_list[file].hfile = NULL;
	return 0;
    }
    r->_errno = EBADF;
    return -1;
}

static _off_t fs_lseek_r(struct _reent *r, int file, _off_t ptr, int dir)
{
}

#endif
