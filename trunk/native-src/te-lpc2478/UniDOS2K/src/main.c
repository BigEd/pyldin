#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include <inttypes.h>
#include "fio.h"
#include "uart.h"
#include "kbd.h"
#include "mci.h"

#include "tffs.h"

#define WAIT { int i; for(i=0; i<800000; i++) asm volatile (" nop "); }

void Pink_Panel(void)
{
    unsigned long  i;
    unsigned short *Ptr16;
    unsigned int   *Ptr32;

    Ptr16 = (unsigned short *)LCD_BUFFER_ADDR;
    Ptr32 = (unsigned int *)LCD_BUFFER_ADDR;

    // *Ptr16 = *PICTURE;
    for(i = 0; i < 76800; i++) {
	*Ptr16++=0xF00F; 
    }
}

void keyboard_handler(uint8_t scan)
{
}

void
_show_dirent(
	dirent_t * pdirent)
{
	printf("%8d byte", pdirent->dir_file_size);
	printf("\t%2d/%02d/%02d - %02d:%02d  ", pdirent->crttime.year,
		pdirent->crttime.month,
		pdirent->crttime.day,
		pdirent->crttime.hour,
		pdirent->crttime.min);

	if (pdirent->dir_attr & DIR_ATTR_DIRECTORY) {
		printf("\033[32m%s\033[0m", pdirent->d_name);
	}
	else {
		printf("%s", pdirent->d_name);
	}
	printf("\n");
}


static tffs_handle_t htffs;

int cmd_mount(int argc, char *argv[])
{
    int ret;

    if ((ret = TFFS_mount("mmc", &htffs)) != TFFS_OK) {
        printf("TFFS_mount() %d\n", ret);
        return -1;
    }

    return 0;
}

int cmd_umount(int argc, char *argv[])
{
    TFFS_umount(htffs);

    return 0;
}

#define MAX_PATH	256
int cmd_ls(int argc, char *argv[])
{
	tdir_handle_t hdir;
	char dir[MAX_PATH];
	int file_num;
	int ret;

	if (argc > 1)
	    strcpy(dir, argv[1]);
	else
	    strcpy(dir, "/");

	if ((ret = TFFS_opendir(htffs, dir, &hdir)) != TFFS_OK) {
		printf("TFFS_opendir %d\n", ret);
		return -1;
	}

	file_num = 0;
	while (1) {
		dirent_t dirent;

		if ((ret = TFFS_readdir(hdir, &dirent)) == TFFS_OK) {
			_show_dirent(&dirent);
		}
		else if (ret == ERR_TFFS_LAST_DIRENTRY) {
			break;
		}
		else {
			printf("TFFS_readdir %d\n", ret);
			break;
		}
		file_num++;
	}

	printf("\nTotal %d files.\n", file_num);
	if ((ret = TFFS_closedir(hdir)) != TFFS_OK) {
		printf("TFFS_closedir %d\n", ret);
		return -1;
	}

	return 0;
}

static const struct commands {
	int (*func)(int argc, char *argv[]);/* function pointer */
	const char *name;	/* name of command */
	const char *arg;	/* brief argument description or NULL */
	const char *desc;	/* brief description printed with "help" */
} commands[] = {
	{ cmd_ls,	"dir",		"path",		"List directory contents" },
	{ cmd_mount,	"mount",	"device",	"Mount device" },
	{ cmd_umount,	"umount",	"device",	"Unmount device" },
	{ 0, 0, 0, 0 }
};

int system(const char *buf)
{
    int i, j, argc;
    int len = strlen(buf);
    char *tmp = (char *)alloca(len);
    strcpy(tmp, buf);

    for (j = 0, argc = 0; j < len; j++) {
	if ((j == 0 && tmp[j] > ' ') || (tmp[j - 1] == 0 && tmp[j] > ' '))
	    argc++;
	
	if (tmp[j] == ' ')
	    tmp[j] = 0;
    }

    char **argv = alloca(argc * sizeof(void));
    for (j = 0, i = 0; i < argc; j++) {
	if ((j == 0 && tmp[j] > ' ') || (tmp[j - 1] == 0 && tmp[j] > ' '))
	    argv[i++] = &tmp[j];
    }

//    show_p(argc, argv);

    for (i = 0; commands[i].func; i++)
	if (!strcmp(argv[0], commands[i].name)) {
	    return commands[i].func(argc, argv);
    }

    printf("Unknown command, \"%s\".\n", buf);

    return 0;
}

#if 0
int testswi(int a, int b, int c)
{
    unsigned long r;
    register long a1 asm ("r0") = (long) a;
    register long a2 asm ("r1") = (long) b;
    register long a3 asm ("r2") = (long) c;
    asm volatile("swi 0x5	\n\t" \
		 "mov %0, r0	\n\t" \
		    : "=r" (r)
		    : "r" (a1), "r" (a2), "r" (a3)
		);
    return r;
}
#endif

static inline int testswi(int a, int b, int c)
{
    register volatile long r asm ("r0");
    register long a1 asm ("r0") = (long) a;
    register long a2 asm ("r1") = (long) b;
    register long a3 asm ("r2") = (long) c;
    asm volatile("swi 0x5 @ %0" \
		    : "=r" (r)
		    : "r" (a1), "r" (a2), "r" (a3)
		);
    return r;
}

int main(void)
{
    uart0Init(UART_BAUD(HOST_BAUD_U0), UART_8N1, UART_FIFO_8); // setup the UART

    uart0Puts("Hello from UART0\r\n");

    Pink_Panel();

    FIOInit(BOARD_LED1_PORT, DIR_OUT, BOARD_LED1_MASK);
    FIOInit(BOARD_LED2_PORT, DIR_OUT, BOARD_LED2_MASK);
    FIOInit(BOARD_LED3_PORT, DIR_OUT, BOARD_LED3_MASK);

    printf("UniDOS 2000\n\n");
//    keyboard_init();

    char *ptr = malloc(1024);
    printf("--%08X %d %p %c\n", testswi(1, 2, 3), sizeof(void*), ptr, 'd');


    FILE *f = fopen("hello.txt", "r");
    f = fopen("hello.txt", "r+");
    f = fopen("hello.txt", "w");
    f = fopen("hello.txt", "w+");
    f = fopen("hello.txt", "a");
    f = fopen("hello.txt", "a+");
    f = fopen("hello.txt", "rb");
    f = fopen("hello.txt", "wb");

    for(;;)
    {
	char buf[128];
	printf("$ ");
	fgets(buf, 128, stdin);
	buf[strlen(buf) - 1] = 0;
	system(buf);
    }

    return 0; 
}

