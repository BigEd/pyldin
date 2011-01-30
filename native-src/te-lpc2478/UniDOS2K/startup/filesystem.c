#ifndef _FILESYSTEM_C_
#define _FILESYSTEM_C_

#include <stdio.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include "../efsl/Inc/disc.h"
#include "../efsl/Inc/partition.h"
#include "../efsl/Inc/fs.h"
#include "../efsl/Inc/file.h"
#include "../efsl/Inc/mkfs.h"
#include "../efsl/Inc/ioman.h"
#include "../efsl/Inc/ui.h"

#define MAX_FILEDESCR	32

typedef struct {
    File *file;
    int used;
} filedesc;

static filedesc fd_list[MAX_FILEDESCR];

static int inited = 0;

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

static hwInterface *lfile=0;
static IOManager *ioman=0;
static Disc *disc=0;
static Partition *part=0;
static FileSystem *fs=0;

static int wrap_fs_mountfs(struct _reent *r, const char *pathname)
{

    lfile = malloc(sizeof(*lfile));
    ioman = malloc(sizeof(*ioman));
    disc = malloc(sizeof(*disc));
    part = malloc(sizeof(*part));

    if_initInterface(lfile,"mci0:");
    ioman_init(ioman,lfile,0);
    disc_initDisc(disc,ioman);
    memClr(disc->partitions,sizeof(PartitionField)*4);
    disc->partitions[0].type=0x0B;
    disc->partitions[0].LBA_begin=0;
    disc->partitions[0].numSectors=lfile->sectorCount;
    part_initPartition(part,disc);

    fs = malloc(sizeof(*fs));
    if( (fs_initFs(fs,part)) != 0) {
	 printf("Unable to init the filesystem\n");
	 return(-1);
    }
    return 0;
}

static int wrap_fs_unmountfs(struct _reent *r, const char *pathname)
{
    fs_umount(fs);
    free(fs);
    return 0;
}

static int wrap_fs_open(struct _reent *r, const char *pathname, int flags, int mode)
{
    wrap_fs_check_is_inited();
    mode = mode;
    int fd = wrap_fs_get_fd();
    if (fd >= 0) {
	euint8 m = 0;
	if (flags == O_RDONLY)
	    m = MODE_READ;
	else if (flags & O_APPEND)
	    m = MODE_APPEND;
	else if (flags & O_WRONLY) {
	    if ((flags & O_CREAT) || (flags & O_TRUNC))
		fs_rmfile(fs, (euint8*)pathname);
	    m = MODE_WRITE;
	}

	fprintf(stderr, "open() flag: %08X mode: %c\n", flags, m);
	if (!m) {
	    fprintf(stderr, "unknown mode\n");
	    r->_errno = EACCES;
	    return -1;
	}

	fd_list[fd].file = (File *)malloc(sizeof(File));
	if (!fd_list[fd].file) {
	    fprintf(stderr, "No memory!\n");
	    r->_errno = ENOMEM;
	    return -1;
	}
	if ( (file_fopen(fd_list[fd].file, fs, (esint8*)pathname, m)) != 0) {
	    fprintf(stderr, "Unable to open the file\n");
	    r->_errno = EACCES;
	    return(-1);
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
	return file_read(fd_list[file].file, len, (euint8*)ptr);
    }
    r->_errno = EBADF;
    return -1;
}

static _ssize_t wrap_fs_write(struct _reent *r, int file, const void *ptr, size_t len)
{
    if (fd_list[file].used) {
	return file_write(fd_list[file].file, len, (euint8*)ptr);
    }
    r->_errno = EBADF;
    return -1;
}

static int wrap_fs_close(struct _reent *r, int file)
{
    if (fd_list[file].used) {
	int ret = 0;
	fprintf(stderr, "close file\n");
	if (file_fclose(fd_list[file].file)) {
	    fprintf(stderr, "f_close %d\n", ret);
	    r->_errno = EIO;
	    ret = -1;
	}
	free(fd_list[file].file);
	fd_list[file].file = NULL;
	fd_list[file].used = 0;
	return 0;
    }
    r->_errno = EBADF;
    return -1;
}

static _off_t wrap_fs_lseek(struct _reent *r, int file, _off_t ptr, int dir)
{
    if (fd_list[file].used) {
	euint32 offset = 0;
	switch (dir) {
	case SEEK_SET:
	    offset = ptr;
	    break;
	case SEEK_CUR:
	    offset = fd_list[file].file->FilePtr + ptr;
	    break;
	case SEEK_END:
	    offset = fd_list[file].file->FileSize + ptr;
	    break;
	default:
	    r->_errno = EINVAL;
	    return -1;
	}
	if (file_setpos(fd_list[file].file, offset)) {
	    r->_errno = EINVAL;
	    return -1;
	}
	return offset;
    }
    r->_errno = EBADF;
    return -1;
}

static int wrap_fs_fstat(struct _reent *r, int file, struct stat *st)
{
    if (fd_list[file].used) {
//	FRESULT ret;
	st->st_size = fd_list[file].file->FileSize;
	st->st_blksize = 512;
	st->st_blocks = ((st->st_size - 1) / 512) + 1;
//	if (s.attr & DIR_ATTR_DIRECTORY)
//	    st->st_mode = S_IFDIR;
//	else
	    st->st_mode = S_IFREG;

//	if (s.attr & DIR_ATTR_READ_ONLY)
//	    st->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
//	else
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
    int rc = fs_mkdir(fs, (eint8*)path);
    if (!rc)
	return 0;

    if (rc == -1) {
	r->_errno = EEXIST;
    } else if (rc == -3) {
	r->_errno = ENOSPC;
    } else
	r->_errno = EACCES;

    return -1;
}

static int wrap_fs_rmdir(struct _reent *r, char *path)
{
    if (!fs_rmfile(fs, (euint8*)path)) {
	return 0;
    }
    r->_errno = EACCES;

    return -1;
}

static int wrap_fs_unlink(struct _reent *r, char *path)
{
    if (!fs_rmfile(fs, (euint8*)path)) {
	return 0;
    }
    r->_errno = EACCES;

    return -1;
}

static void *wrap_fs_opendir(struct _reent *r, char *path)
{
#if 0
    DIR *d = (DIR *)malloc(sizeof(DIR));
    if (!d) {
	r->_errno = ENOMEM;
	return NULL;
    }
    FRESULT rc = f_opendir(d, path);
    switch (rc) {
    case FR_OK: return d;
    case FR_NO_PATH:
    case FR_INVALID_NAME:
    case FR_INVALID_DRIVE:
	r->_errno = ENOENT;
	break;
    default:
	r->_errno = EACCES;
    }
#endif
    return NULL;
}

static int wrap_fs_readdir_r(struct _reent *r, void *dirp, struct dirent *entry, struct dirent **result)
{
#if 0
    FILINFO fno;
    FRESULT rc = f_readdir(dirp, &fno);
    if (rc == FR_OK) {
	char *fn;
	if (fno.fname[0] == 0) {
	    *result = NULL;
	    return 0;
	}
#if 0
#if _USE_LFN
	fn = *fno.lfname ? fno.lfname : fno.fname;
#else
	fn = fno.fname;
#endif
#endif
	fn = fno.fname;
	strncpy(entry->d_name, fn, NAME_MAX - 1);
	entry->d_type = 0;
	if (fno.fattrib & AM_DIR)
	    entry->d_type |= DT_DIR;
	else
	    entry->d_type |= DT_REG;
	entry->d_size = fno.fsize;
	*result = entry;
	return 0;
    }
#endif
    r->_errno = EBADF;
    *result = NULL;
    return -1;
}

static int wrap_fs_closedir(struct _reent *r, void *dirp)
{
#if 0
#warning "SWI wrap_fs_closedir"
    free(dirp);
    return 0;
#endif
}

#endif
