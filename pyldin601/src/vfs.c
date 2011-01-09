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

static int vfs_enabled = 0;
static int vfs_drive = 4; // Drive E:

static char *root_dir = "/tmp";
static char cur_dir[PATH_MAX];
static DIR *dir = NULL;
static int en_mask = 0;
static int di_mask = 0;

static void copy_to_mem(word to, char *from, word size)
{
    while (size--)
	mc6800_memw(to++, *from++);
}

static void copy_from_mem(char *to, word from, word size)
{
    while (size--)
	*to++ = mc6800_memr(from++);
}

static int convert_to_dos_name(word to, char *from)
{
    int p = 0;
    for (p = 0; p < 11; p++)
	mc6800_memw(to + p, ' ');
    for (p = 0; p < 11; p++) {
	char c = *from++;
	if (c == 0)
	    break;
	if (c == '.') {
	    p = 7;
	    continue;
	}
	mc6800_memw(to + p, c);
    }
    return 0;
}

int read_dir(DIR *dir, word tmp, int en, int di)
{
    struct dirent *d = NULL;
    struct stat st;
    char f[256];
    while (1) {
	d = readdir(dir);
	if (!d) {
	    closedir(dir);
	    dir = NULL;
	    return 0x0b;
	}
	if (d->d_type & DT_DIR) {
	    if (di & 0x10)
		continue;
	} else {
	    if (en & 0x10)
		continue;
	}
	if (d->d_name[0] == '.')
	    continue;
	break;
    }

    convert_to_dos_name(tmp, d->d_name);
    mc6800_memw(tmp + 11, (d->d_type & DT_DIR)?0x10:0x0);
    snprintf(f, 256, "%s/%s", cur_dir, d->d_name);
    if (!stat(f, &st)) {
	struct tm *dt;
	word t;

	mc6800_memw(tmp + 28, (st.st_size      ) & 0xff);
	mc6800_memw(tmp + 29, (st.st_size >>  8) & 0xff);
	mc6800_memw(tmp + 30, (st.st_size >> 16) & 0xff);
	mc6800_memw(tmp + 31, (st.st_size >> 24) & 0xff);

	dt = localtime(&st.st_mtime);
	
	t = dt->tm_mday | ((dt->tm_mon + 1) << 5) | ((dt->tm_year - 80) << 9);
	mc6800_memw(tmp + 24, t & 0xff);
	mc6800_memw(tmp + 25, t >> 8);

	t = (dt->tm_sec / 2) | (dt->tm_min << 5) | (dt->tm_hour << 11);
	mc6800_memw(tmp + 22, t & 0xff);
	mc6800_memw(tmp + 23, t >> 8);
    }

    return 0;
}

int SWIemulator(int swi, byte *A, byte *B, word *X, byte *t, word *PC)
{
    switch(swi) {
    case 0xFE: {
	fprintf(stderr, "INT $FE = %02X %02X %04X\n", *A, *B, *X);
	strcpy(cur_dir, root_dir);
	*A = 0;
	return 1;
	}
	break;
    case 0x42: {
	word tmp = (mc6800_memr(*X + 4) << 8) + mc6800_memr(*X + 5);
	char c = 0;
	fprintf(stderr, "INT $42 (%02X %02X) = ", *A, *B);
	while ((c = mc6800_memr(tmp++)))
	    fprintf(stderr, "%c", c);
	fprintf(stderr, "\n");
	}
	if (vfs_enabled) {
	    word tmp = (mc6800_memr(*X + 2) << 8) + mc6800_memr(*X + 3);
	    en_mask = *A;
	    di_mask = *B;
	    if ((*A == 0x08) && (*B == 0x00)) {
		copy_to_mem(tmp, "HOSTFS_P601", 11);
		mc6800_memw(tmp + 11, 0x08);
		*A = 0;
	    } else if ((*A == 0x10) && (*B == 0x88)) {
		//  copy_to_mem(tmp, "           ", 11);
		copy_to_mem(tmp, "hello      ", 11);
		mc6800_memw(tmp + 11, 0x10);
		*A = 0;
	    } else {
		dir = opendir(cur_dir);
		*A = read_dir(dir, tmp, en_mask, di_mask);
	    }
	    return 1;
	}
	break;
    case 0x43:
	fprintf(stderr, "INT $43\n");
	if (vfs_enabled) {
	    word tmp = (mc6800_memr(*X + 2) << 8) + mc6800_memr(*X + 3);
	    *A = read_dir(dir, tmp, en_mask, di_mask);
	    return 1;
	}
	break;
    case 0x44:
	fprintf(stderr, "set drive=%d\n", *A);
	if (*A == vfs_drive) {
	    vfs_enabled = 1;
	    *A = 0;
	    return 1;
	} else
	    vfs_enabled = 0;
	break;
    case 0x45:
	fprintf(stderr, "get drive\n");
	if (vfs_enabled) {
	    *A = vfs_drive;
	    return 1;
	}
	break;
    case 0x46:
	fprintf(stderr, "INT $46 - ");
	if (vfs_enabled) {
	    struct stat st;
	    char path[PATH_MAX];
	    char path_new[PATH_MAX];
	    char c;
	    word tmp = *X;
	    char *ptr;
	    snprintf(path, PATH_MAX, "%s/", cur_dir);
	    ptr = path;
	    while(*++ptr) ;
	    while ((c = mc6800_memr(tmp++))) {
		if (c == '\\')
		    c = '/';
		*ptr++ = c;
	    }
	    *ptr = 0;
	    fprintf(stderr, "%s -- ", path);
	    realpath(path, path_new);
	    fprintf(stderr, "%s\n", path_new);
	    if (stat(path_new, &st)) {
		*A = 0x0b;
		return 1;
	    }
	    strcpy(cur_dir, path_new);
	    *A = 0;
	    return 1;
	}
	break;
    case 0x47:
	fprintf(stderr, "INT $47\n");
	if (*A == vfs_drive) {
	    int p = strlen(root_dir);
	    char c = 0;
	    int p1 = 0;
	    while ((c = cur_dir[p++])) {
		if (c == '/')
		    c = '\\';
		mc6800_memw(*X + p1++, c);
	    }
	    mc6800_memw(*X + p1, 0);
	    *A = 0;
	    return 1;
	}
	break;
    case 0x52:
	if (*A == vfs_drive) {
	    *A = 0;
	    return 1;
	}
    }
    return 0;
}

