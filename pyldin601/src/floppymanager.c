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
#include "screen.h"
#include "floppy.h"

#ifdef _EE
#include "ps2func.h"
#endif

//floppy
static char	*filename[4];
static char	*datadir;
static struct dirent **namelist;
static int 	namecount = 0;
static int 	wantDisk = 0;
static int	name_selected = 2;

#if defined(_WIN32) || defined(_PSP)
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
#elif defined(_EE)
static int getdir(char *path, struct dirent ***namelist)
{
    struct dirent   **names = NULL;
    int 		count = 0;

    if (!strncmp(path, "cdfs", 4)) {
	struct TocEntry myTocEntries[128];
	char pathname[1024];
	int i;
	
	strcpy(pathname, path + 5);

	CDVD_Init();

	count = CDVD_GetDir(pathname, NULL, CDVD_GET_FILES_ONLY, myTocEntries, 128, pathname);

	names = (struct dirent **) malloc((count + 2) * sizeof(struct dirent *));

	names[0] = NULL;
	names[1] = NULL;

	for (i = 0; i < count; i++) {
	    names[i + 2] = (struct dirent *) malloc (sizeof(struct dirent));
	    strncpy(names[i + 2]->d_name, myTocEntries[i].filename, 256);
	}

	count += 2;

	CDVD_Stop();	
    } else {
	fio_dirent_t 	dir;
	int 		ret;
	int 		fd = fioDopen(path);

	while( (ret=fioDread( fd, &dir ))>0 ) {
	    struct dirent **tmp;
	
	    tmp = (struct dirent **) malloc((count + 1) * sizeof(struct dirent *));
	
	    memcpy(tmp, names, count * sizeof(struct dirent *));
	
	    tmp[count] = (struct dirent *) malloc (sizeof(struct dirent));
	
	    strncpy(tmp[count]->d_name, dir.name, 256);
	
	    if (names)
		free(names);
	
	    names = tmp;

    	    //printf( "%s | %d | %d\n",dir.name, dir.stat.mode, dir.stat.size );
	
    	    count++;
	}
    
	fioDclose(fd);
    }

    *namelist = names;
    
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

#ifdef _EE
    if (!strncmp(path, "cdfs", 4))
	CDVD_Init();
#endif

    if (loadDiskImage(filename[disk], disk) < 0) {
	free(filename[disk]);
	filename[disk] = NULL;
	fprintf(stderr, "Failed\n");
    } else 
	fprintf(stderr, "Ok\n");

#ifdef _EE
    if (!strncmp(path, "cdfs", 4))
	CDVD_Stop();
#endif
}

void FloppyManagerOn(int disk, char *dir)
{
    char dirn[256];

    datadir = dir;

    wantDisk = disk;

    sprintf(dirn, "%s/Floppy", datadir);

#if defined(_WIN32) || defined(_EE) || defined(_PSP)
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
    drawString(0,0,"Select floppy image:", 0xffff, 0);
    drawString(0,29,"Cancel", 0xffff, 0);

    for (i = 2; i < namecount; i++) {
	if (i == name_selected)
	    drawString(0, i + 1, namelist[i]->d_name, 0, 0xffff);
	else
	    drawString(0, i + 1, namelist[i]->d_name, 0xffff, 0);
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
