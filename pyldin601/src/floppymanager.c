#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#if defined(__APPLE__) && (__GNUC__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include "floppymanager.h"
#include "pyldin.h"
#include "floppy.h"
#include "discs.h"

//floppy
static char	*filename[4];
static char	*datadir;
static struct dirent **namelist;
static int 	namecount = 0;
static int 	wantDisk = 0;
static int	name_selected = 2;

#if defined(_WIN32)
static int getdir(char *path, struct dirent ***namelist)
{
    int 		count = 0;
    struct dirent   	**names = NULL;

    struct dirent   	*dit;

    DIR             	*dip;

    if ((dip = opendir(path)) == NULL) {
        return 0;
    }

    while ((dit = readdir(dip)) != NULL) {
	struct dirent **tmp;
	tmp = (struct dirent **) malloc((count + 1) * sizeof(struct dirent *));
	memcpy(tmp, names, count * sizeof(struct dirent *));
	tmp[count] = (struct dirent *) malloc (sizeof(struct dirent));
	memcpy(tmp[count], dit, sizeof(struct dirent));
	if (names)
	    free(names);
	names = tmp;
        count++;
    }

    *namelist = names;
    closedir(dip);

    return count;
}
#endif

void initFloppy(void)
{
    int i;
    for (i = 0; i < 4; i++)
	filename[i] = NULL;

    wantDisk = 0;
}

void freeFloppy(void)
{
    int i;
    for (i = 0; i < 4; i++) {
	if (filename[i] != NULL)
	    ejectFloppy(i);
    }
}

void ejectFloppy(int disk)
{
    fprintf(stderr, "Eject floppy: %s ... ", filename[disk]);

    if (unloadDiskImage(filename[disk], disk) < 0) {
	fprintf(stderr, "Failed\n");
    } else
	fprintf(stderr, "Ok\n");

    free(filename[disk]);
    filename[disk] = NULL;
}

void insertFloppy(int disk, char *path)
{
    if (filename[disk])
	ejectFloppy(disk);

    filename[disk] = (char *) malloc(strlen(path) + 1);

    strcpy(filename[disk], path);

    fprintf(stderr, "Insert floppy: %s ... ", filename[disk]);

    if (loadDiskImage(filename[disk], disk) < 0) {
	free(filename[disk]);
	filename[disk] = NULL;
	fprintf(stderr, "Failed\n");
    } else 
	fprintf(stderr, "Ok\n");
}

void FloppyManagerOn(int disk, char *dir)
{
    char dirn[256];

    datadir = dir;

    wantDisk = disk;

    sprintf(dirn, "%s/Floppy", datadir);

#if defined(_WIN32)
    namecount = getdir(dirn, &namelist);
#else
    namecount = scandir(dirn, &namelist, 0, alphasort);
#endif

    name_selected = 2;

    FloppyManagerUpdateList(0);
}

void FloppyManagerUpdateList(int dir) 
{
    int i;

    name_selected += dir;

    if (name_selected < 2)
	name_selected = 2;

    if (name_selected >= namecount)
	name_selected = namecount - 1;

    clearScr();
    drawString("Select floppy image:", 0, 0, 0xffff, 0);
    drawString("Cancel", 0, 29, 0xffff, 0);

    for (i = 2; i < namecount; i++) {
	if (i == name_selected)
	    drawString(namelist[i]->d_name, 0, i + 1, 0, 0xffff);
	else
	    drawString(namelist[i]->d_name, 0, i + 1, 0xffff, 0);
    }
}

void FloppyManagerOff(void)
{
    int i;
    for (i = 0; i < namecount; i++) {
	if (namelist[i])
	    free(namelist[i]);
    }
    free(namelist);
}

int selectFloppy(int y)
{
    y /= 8;
	
    y -= 1;

    if (y >= namecount) 
	return 0;
	
    if (y < 2) 
	return 0;

    char buf[256];

    sprintf(buf, "%s/Floppy/%s", datadir, namelist[y]->d_name);

    insertFloppy(wantDisk, buf);

    return 0;
}

int selectFloppyByNum()
{
    int n = name_selected;

    if (n >= namecount) 
	return 0;
	
    if (n < 2) 
	return 0;

    char buf[256];

    sprintf(buf, "%s/Floppy/%s", datadir, namelist[n]->d_name);

    insertFloppy(wantDisk, buf);

    return 0;
}
