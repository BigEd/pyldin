#ifndef _FILESYSTEM_C_
#define _FILESYSTEM_C_

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
    mode = mode;
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

static _off_t fs_lseek(struct _reent *r, int file, _off_t ptr, int dir)
{
    if (fd_list[file].hfile) {
	int d;
	switch (dir) {
	case SEEK_SET:
	    d = TFFS_SEEK_SET;
	    break;
	case SEEK_CUR:
	    d = TFFS_SEEK_CUR;
	    break;
	case SEEK_END:
	    d = TFFS_SEEK_END;
	    break;
	default:
	    r->_errno = EINVAL;
	    return -1;
	}
	_off_t ret = TFFS_fseek(fd_list[file].hfile, ptr, d);
	if (ret < 0) {
	    r->_errno = EINVAL;
	    ret = -1;
	}
	return ret;
    }
    r->_errno = EBADF;
    return -1;
}

static int fs_fstat(struct _reent *r, int file, struct stat *st)
{
    if (fd_list[file].hfile) {
	int ret;
	tffs_stat_t s;
	if ((ret = TFFS_fstat(fd_list[file].hfile, &s)) != TFFS_OK) {
	    fprintf(stderr, "TFFS_fstat %d\n", ret);
	    r->_errno = EBADF;
	    return -1;
	}
	st->st_size = s.size;
	st->st_blksize = 512;
	st->st_blocks = ((s.size - 1) / 512) + 1;
	if (s.attr & DIR_ATTR_DIRECTORY)
	    st->st_mode = S_IFDIR;
	else
	    st->st_mode = S_IFREG;
	if (s.attr & DIR_ATTR_READ_ONLY)
	    st->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
	else
	    st->st_mode |= (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	return 0;
    }
    r->_errno = EBADF;
    return -1;
}

static int fs_isatty(int file)
{
    if (fd_list[file].hfile) {
	errno = EINVAL;
	return 0;
    }

    errno = EBADF;
    return 0;
}

#endif
