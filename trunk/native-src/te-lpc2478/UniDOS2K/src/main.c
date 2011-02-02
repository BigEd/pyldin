#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "dirent.h"

#include "elf.h"

int cmd_mount(int argc, char *argv[])
{
    int ret;

    if (!mount_fs("mmc:"))
	fprintf(stderr, "Mounted!\n");
    else
	fprintf(stderr, "Unable mount.\n");

    return 0;
}

int cmd_umount(int argc, char *argv[])
{
    umount_fs("mmc:");
    return 0;
}

#define MAX_PATH	256
int cmd_ls(int argc, char *argv[])
{
    DIR *dirp;
    struct dirent *dp;
    char dir[MAX_PATH];

    if (argc > 1)
	strcpy(dir, argv[1]);
    else
	strcpy(dir, ".");

    if ((dirp = opendir(dir)) == NULL) {
	fprintf(stderr, "couldn't open directory.\n");
	return -1;
    }

    while ((dp = readdir(dirp)) != NULL) {
	if (dp->d_type & DT_DIR)
	    printf("   <dir>  %s\n", dp->d_name);
	else
	    printf("%8lu  %s\n", dp->d_size, dp->d_name);
    }

    closedir(dirp);

    return 0;
}

//#include <fcntl.h>
int cmd_type(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No source file!\n");
	return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (f) {
	char buf[1024];
	int total = 0;
	int ret;
	while ((ret = fread(buf, 1, sizeof(buf), f)) > 0) {
	    fwrite(buf, 1, ret, stdout);
	    total += ret;
	}
	fclose(f);
    } else
	fprintf(stderr, "No such file!\n");

    return 0;
}

int cmd_md(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No directory!\n");
	return -1;
    }

    if (mkdir(argv[1], 0)) {
	fprintf(stderr, "Unable create directory.\n");
	return -1;
    }

    return 0;
}

int cmd_rd(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No directory!\n");
	return -1;
    }

    if (rmdir(argv[1])) {
	fprintf(stderr, "Unable remove directory.\n");
	return -1;
    }

    return 0;
}

int cmd_del(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No such file!\n");
	return -1;
    }

    if (unlink(argv[1])) {
	fprintf(stderr, "Unable remove file.\n");
	return -1;
    }

    return 0;
}

int cmd_rename(int argc, char *argv[])
{
    if (argc < 3) {
	fprintf(stderr, "No such file!\n");
	return -1;
    }

    if (rename(argv[1], argv[2])) {
	fprintf(stderr, "Unable remove file.\n");
	return -1;
    }

    return 0;
}

int cmd_cd(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No new path!\n");
	return -1;
    }

    if (chdir(argv[1])) {
	fprintf(stderr, "Unable change directory.\n");
	return -1;
    }

    return 0;
}

int cmd_exec(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "No source file!\n");
	return -1;
    }

    if (exec_elf(argv[1]) != 0) {
	fprintf(stderr, "Error load or execute!\n");
    }
    return 0;
}

int cmd_run(const char *cmdline)
{
    return exec_elf((char *)cmdline);
}

int cmd_dump(int argc, char *argv[])
{
    if (argc < 3) {
	fprintf(stderr, "No start address and length!\n");
	return -1;
    }

    uint8_t *base = (uint8_t *) strtoul(argv[1], 0, 0);
    uint32_t len = strtoul(argv[2], 0, 0);
    uint32_t i;
    char s[17];

    for (i = 0; i < len; i++) {
	if (i % 16 == 0) {
	    if (!i)
		printf("\n%08X: ", base + i);
	    else
		printf("%s\n%08X: ", s, base + i);
	    memset(s, 0, 17);
	}
	printf("%02X ", base[i]);
	s[i % 16] = ((base[i] >= 32) && (base[i] < 128))?base[i]:'.';
    }

    printf("\n");

    return 0;
}

int cmd_heap(int argc, char *argv[])
{
    extern char *heap_ptr;
    printf("Current heap address: %p\n", heap_ptr);

    return 0;
}

static const struct commands {
	int (*func)(int argc, char *argv[]);/* function pointer */
	const char *name;	/* name of command */
	const char *arg;	/* brief argument description or NULL */
	const char *desc;	/* brief description printed with "help" */
} commands[] = {
	{ cmd_mount,	"mount",	"device",	"Mount device" },
	{ cmd_umount,	"umount",	"device",	"Unmount device" },
	{ cmd_ls,	"dir",		"path",		"List directory contents" },
	{ cmd_type,	"type",		"file",		"Type file" },
	{ cmd_md,	"md",		"dir",		"Make directory" },
	{ cmd_rd,	"rd",		"dir",		"Remove directory" },
	{ cmd_del,	"del",		"file",		"Remove file" },
	{ cmd_rename,	"rename",	"old new",	"Rename file" },
	{ cmd_cd,	"cd",		"newdir",	"Change directory" },
	{ cmd_exec,	"exec",		"file",		"Execute elf file" },
	{ cmd_dump,	"dump",		"addr offset",	"Show memory" },
	{ cmd_heap,	"heap",		"",		"Show heap" },
	{ 0, 0, 0, 0 }
};

int system(const char *buf)
{
    int i, j, argc;
    int len = strlen(buf);
    char *tmp = (char *)alloca(len + 1);
    strcpy(tmp, buf);

    for (j = 0, argc = 0; j < len; j++) {
	if ((j == 0 && tmp[j] > ' ') || (tmp[j - 1] == 0 && tmp[j] > ' '))
	    argc++;
	
	if (tmp[j] == ' ')
	    tmp[j] = 0;
    }

    if (!argc)
	return 0;

    char **argv = alloca(argc * sizeof(void));
    for (j = 0, i = 0; i < argc; j++) {
	if ((j == 0 && tmp[j] > ' ') || (tmp[j - 1] == 0 && tmp[j] > ' '))
	    argv[i++] = &tmp[j];
    }

    for (i = 0; commands[i].func; i++)
	if (!strcmp(argv[0], commands[i].name)) {
	    return commands[i].func(argc, argv);
    }

    if (cmd_run(buf))
	printf("Bad command or file name.\n");

    return 0;
}

int main(void)
{
    printf("UniDOS 2000\n\n");

    for(;;)
    {
	char buf[128];
	char wd[128];
	getcwd(wd, 128);
	printf("%s $ ", wd);
	fgets(buf, 128, stdin);
	buf[strlen(buf) - 1] = 0;
	system(buf);
    }

    return 0;
}

