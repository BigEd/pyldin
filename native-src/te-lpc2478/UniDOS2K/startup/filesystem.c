#ifndef _FILESYSTEM_C_
#define _FILESYSTEM_C_

#include <stdio.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include "fullfat.h"
#include "blkdev_mci.h"

#define PARTITION_NUMBER	0					// FullFAT can mount primary partitions only. Specified at Runtime.

#define MAX_FILEDESCR		32

typedef struct {
    FF_FILE *file;
    int used;
} filedesc;

static filedesc fd_list[MAX_FILEDESCR];

static int inited = 0;

static char current_path[256];

static void wrap_fs_check_is_inited(void)
{
    int i;
    if (!inited) {
	// reserved fds
	fd_list[0].used = 1; // stdin
	fd_list[1].used = 1; // stdout
	fd_list[2].used = 1; // stderr
	for (i = 3; i < MAX_FILEDESCR; i++)
	    fd_list[i].used = 0;
	inited = 1;
    }
}

static int wrap_fs_get_fd(void)
{
    int i;
    for (i = 0; i < MAX_FILEDESCR; i++) {
	if (!fd_list[i].used)
	    return i;
    }
    return -1;
}

static BLK_DEV_MCI	hDisk = NULL;
static FF_IOMAN		*pIoman = NULL;

static int wrap_fs_mountfs(struct _reent *r, const char *pathname)
{
    FF_ERROR Error = FF_ERR_NONE;

    if (hDisk) {
	r->_errno = EBUSY;
	return -1;
    }

    hDisk = fnOpen("mci");
    if (!hDisk) {
	r->_errno = EACCES;
	return -1;
    }

    pIoman = FF_CreateIOMAN(NULL, 4096, GetBlockSize(hDisk), &Error);	// Using the BlockSize from the Device Driver (see blkdev_win32.c)
    if (!pIoman) {
	fnClose(hDisk);
	hDisk = NULL;
	r->_errno = EACCES;
	return -1;
    }

    Error = FF_RegisterBlkDevice(pIoman, GetBlockSize(hDisk), (FF_WRITE_BLOCKS) fnWrite, (FF_READ_BLOCKS) fnRead, hDisk);
    if (Error) {
	printf("Error Registering Device\nFF_RegisterBlkDevice() function returned with Error %ld.\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
    }

    Error = FF_MountPartition(pIoman, PARTITION_NUMBER);
    if (Error) {
	fnClose(hDisk);
	FF_DestroyIOMAN(pIoman);
	fprintf(stderr, "FullFAT Couldn't mount the specified parition!\n");
	fprintf(stderr, "FF_MountPartition() function returned with Error %ld\nFullFAT says: %s\n", Error, FF_GetErrMessage(Error));
	pIoman = NULL;
	hDisk = NULL;
	r->_errno = EACCES;
	return -1;
    }

    strcpy(current_path, "/");

    return 0;
}

static int wrap_fs_unmountfs(struct _reent *r, const char *pathname)
{
    if (!hDisk) {
	r->_errno = EINVAL;
	return -1;
    }

    FF_UnmountPartition(pIoman);						// Dis-mount the mounted partition from FullFAT.
    FF_DestroyIOMAN(pIoman);							// Clean-up the FF_IOMAN Object.
    fnClose(hDisk);
    hDisk = NULL;
    pIoman = NULL;

    return 0;
}

static char *normalize_path(char *path)
{
    int i = 0;
    int len = strlen(path);
    while (path[i]) {
	if (path[i] == '/') {
	    if (path[i + 1] == '/') {
		memcpy(&path[i], &path[i + 1], len - i);
		continue;
	    }
	    if (path[i + 1] == '.') {
		if (path[i + 2] == '/' || path[i + 2] == 0) {
		    memcpy(&path[i + 1], &path[i + 2], len - i - 1);
		    continue;
	        }
	        if (path[i + 2] == '.') {
		    if ((path[i + 3] == '/') || (path[i + 3] == 0)) {
			int j = i + 3;
			for (; i > 0 && path[i - 1] != '/'; i--);
			if (i > 0)
			    i--;
			memcpy(&path[i + 1], &path[j], len - j + 1);
			continue;
		    }
	        }
	    }
	}
	i++;
    }
//    if (i > 1 && path[i - 1] == '/')
//	path[i - 1] = 0;
    return path;
}

static int wrap_fs_open(struct _reent *r, const char *pathname, int flags, int mode)
{
    wrap_fs_check_is_inited();
    mode = mode;
    int fd = wrap_fs_get_fd();
    if (fd >= 0) {
	FF_ERROR error;
	FF_T_UINT8 m = 0;
	flags &= 0xffff;
	if ((flags & O_ACCMODE) == O_RDONLY)
	    m |= FF_MODE_READ;
	if ((flags & O_ACCMODE) & O_WRONLY)
	    m |= FF_MODE_WRITE;
	if ((flags & O_ACCMODE) & O_RDWR)
	    m |= (FF_MODE_READ | FF_MODE_WRITE);
	if (flags & O_APPEND)
	    m |= FF_MODE_APPEND;
	if (flags & O_CREAT)
	    m |= FF_MODE_CREATE;
	if (flags & O_TRUNC)
	    m |= FF_MODE_TRUNCATE;

	char tmpath[256];
	if ((pathname[0] != '/') && (pathname[0] != '\\'))
	    snprintf(tmpath, 256, "%s/%s", current_path, pathname);
	else
	    strcpy(tmpath, pathname);
	normalize_path(tmpath);

	fprintf(stderr, "open(%s) flag: %08X mode: %02X\n", tmpath, flags, m);

	if ((fd_list[fd].file = FF_Open(pIoman, tmpath, m, &error)) == NULL) {
	    r->_errno = EACCES;
	    return -1;
	}

	fd_list[fd].used = 1;
	return fd;
    }
    r->_errno = ENFILE;
    return -1;
}

static _ssize_t wrap_fs_read(struct _reent *r, int file, void *ptr, size_t len)
{
    if (fd_list[file].used) {
	return FF_Read(fd_list[file].file, len, 1, ptr);
    }
    r->_errno = EBADF;
    return -1;
}

static _ssize_t wrap_fs_write(struct _reent *r, int file, void *ptr, size_t len)
{
    if (fd_list[file].used) {
	return FF_Write(fd_list[file].file, len, 1, ptr);
    }
    r->_errno = EBADF;
    return -1;
}

static int wrap_fs_close(struct _reent *r, int file)
{
    if (fd_list[file].used) {
	int ret = 0;
	fprintf(stderr, "close file\n");
	FF_ERROR error = FF_Close(fd_list[file].file);
	if (error) {
	    fprintf(stderr, "f_close %d\n", ret);
	    r->_errno = EIO;
	    ret = -1;
	}
	fd_list[file].used = 0;
	return 0;
    }
    r->_errno = EBADF;
    return -1;
}

static _off_t wrap_fs_lseek(struct _reent *r, int file, _off_t ptr, int dir)
{
    if (fd_list[file].used) {
	FF_T_INT8 origin;
	switch (dir) {
	case SEEK_SET:
	    origin = FF_SEEK_SET;
	    break;
	case SEEK_CUR:
	    origin = FF_SEEK_CUR;
	    break;
	case SEEK_END:
	    origin = FF_SEEK_END;
	    break;
	default:
	    r->_errno = EINVAL;
	    return -1;
	}

	if (FF_Seek(fd_list[file].file, ptr, origin)) {
	    r->_errno = EINVAL;
	    return -1;
	}
	return FF_Tell(fd_list[file].file);
    }
    r->_errno = EBADF;
    return -1;
}

static int wrap_fs_fstat(struct _reent *r, int file, struct stat *st)
{
    if (fd_list[file].used) {
	st->st_size = fd_list[file].file->Filesize;
	st->st_blksize = 512;
	st->st_blocks = ((st->st_size - 1) / 512) + 1;
	st->st_mode = S_IFREG;
	st->st_mode |= (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	return 0;
    }
    r->_errno = EBADF;
    return -1;
}

static int wrap_fs_isatty(int file)
{
    if (fd_list[file].used) {
	errno = EINVAL;
	return 0;
    }

    errno = EBADF;
    return 0;
}

static int wrap_fs_mkdir(struct _reent *r, char *path, uint32_t mode)
{
    char tmpath[256];
    if ((path[0] != '/') && (path[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, path);
    else
	strcpy(tmpath, path);
    normalize_path(tmpath);

    FF_ERROR error = FF_MkDir(pIoman, tmpath);

    if (!error)
	return 0;

    fprintf(stderr, "FullFAT says: (%d) %s\n", error, FF_GetErrMessage(error));;

    if (error == FF_ERR_DIR_OBJECT_EXISTS) {
	r->_errno = EEXIST;
    } else
	r->_errno = EACCES;

    return -1;
}

static int wrap_fs_rmdir(struct _reent *r, char *path)
{
    char tmpath[256];
    if ((path[0] != '/') && (path[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, path);
    else
	strcpy(tmpath, path);
    normalize_path(tmpath);

    FF_ERROR error = FF_RmDir(pIoman, tmpath);

    if (!error)
	return 0;

    r->_errno = EACCES;

    return -1;
}

static int wrap_fs_unlink(struct _reent *r, char *path)
{
    char tmpath[256];
    if ((path[0] != '/') && (path[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, path);
    else
	strcpy(tmpath, path);
    normalize_path(tmpath);

fprintf(stderr, "unlink(%s)\n", tmpath);

    FF_ERROR error = FF_RmFile(pIoman, tmpath);

    if (!error)
	return 0;

    fprintf(stderr, "FullFAT says: (%d) %s\n", error, FF_GetErrMessage(error));;

    r->_errno = EACCES;

    return -1;
}

static int wrap_fs_rename(struct _reent *r, char *oldpath, char *newpath)
{
    char tmpath[256];
    if ((oldpath[0] != '/') && (oldpath[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, oldpath);
    else
	strcpy(tmpath, oldpath);
    normalize_path(tmpath);

    char tmpath1[256];
    if ((newpath[0] != '/') && (newpath[0] != '\\'))
	snprintf(tmpath1, 256, "%s/%s", current_path, newpath);
    else
	strcpy(tmpath1, newpath);
    normalize_path(tmpath1);

    FF_ERROR error = FF_Move(pIoman, tmpath, tmpath1);

    if (!error)
	return 0;

    r->_errno = EACCES;

    return -1;
}

static char *wrap_fs_getcwd(struct _reent *r, char *buf, int size)
{
    if (buf && !size) {
	r->_errno = EINVAL;
	return NULL;
    }
    if (!buf)
	buf = malloc(size);
    if (!buf) {
	r->_errno = EINVAL;
	return NULL;
    }

    strncpy(buf, current_path, size - 1);
    return buf;
}

static int wrap_fs_chdir(struct _reent *r, char *path)
{
    FF_DIRENT dir;
    char tmpath[256];
    if ((path[0] != '/') && (path[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, path);
    else
	strcpy(tmpath, path);
    normalize_path(tmpath);

fprintf(stderr, "chdir(%s)\n", tmpath);

    if (!FF_FindFirst(pIoman, &dir, tmpath)) {
	strcpy(current_path, tmpath);
	return 0;
    }

    r->_errno = ENOTDIR;
    return -1;
}

typedef struct {
    FF_DIRENT	dirent;
    int skip;
} FF_DIR;

static void *wrap_fs_opendir(struct _reent *r, char *path)
{
    FF_DIR *dir;
    FF_ERROR error;

    dir = malloc(sizeof(FF_DIR));
    if (!dir) {
	r->_errno = ENOMEM;
	return NULL;
    }

    char tmpath[256];
    if ((path[0] != '/') && (path[0] != '\\'))
	snprintf(tmpath, 256, "%s/%s", current_path, path);
    else
	strcpy(tmpath, path);
    normalize_path(tmpath);

fprintf(stderr, "opendir(%s)\n", tmpath);

    error = FF_FindFirst(pIoman, &dir->dirent, tmpath);
    if (!error) {
	dir->skip = 1;
	return dir;
    }
    free(dir);

    if (errno == -2)
	r->_errno = ENOENT;
    else
	r->_errno = EACCES;
    return NULL;
}

static int wrap_fs_readdir_r(struct _reent *r, void *dirp, struct dirent *entry, struct dirent **result)
{
    FF_DIR *dir = dirp;

    if (dir->skip)
	dir->skip = 0;
    else {
	FF_ERROR error = FF_FindNext(pIoman, &dir->dirent);
	if (error) {
	    *result = NULL;
	    return 0;
	}
    }

    strcpy(entry->d_name, dir->dirent.FileName);
    entry->d_size = dir->dirent.Filesize;
    entry->d_type = 0;
    if (dir->dirent.Attrib & FF_FAT_ATTR_DIR)
	entry->d_type |= DT_DIR;
    else
	entry->d_type |= DT_REG;

    *result = entry;
    return 0;
}

static int wrap_fs_closedir(struct _reent *r, void *dirp)
{
    free(dirp);
    return 0;
}

#endif
