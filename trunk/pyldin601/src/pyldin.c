/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <zlib.h>
#include <time.h>

#include <SDL.h>

#include "pyldin.h"
#include "mc6800.h"
#include "screen.h"
#include "keyboard.h"
#include "floppy.h"
#include "wave.h"
#include "sshot.h"
#include "printer.h"
#include "floppymanager.h"

#include "kbdfix.h"

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifdef USE_JOYSTICK
#include "joyfix.h"
#ifdef USE_JOYMOUSE
static int joymouse_enabled = 0;
static int joymouse_x = 160;
static int joymouse_y = 110;
#endif
#endif

char *datadir = DATADIR;

#define MAX_LOADSTRING 100

static SDL_Surface *screen;

//
static int updateScreen = 0;
static int show_info = 0;
static volatile uint64_t actual_speed = 0;
//
int	filemenuEnabled = 0;
// timer
int	tick50 = 0;
int	fRef = 0;
int	curBlink = 0;

//
static volatile uint64_t one_takt_delay = 0;
static volatile uint64_t one_takt_calib = 0;
static volatile uint64_t one_takt_one_percent = 0;

#if defined(__PPC__)
#warning "PPC32 code"

#define TIMEBASE 79800000

#define READ_TIMESTAMP(val) \
{				\
    unsigned int tmp;		\
    do {			\
	__asm__ __volatile__ (	\
	"mftbu %0 \n\t"		\
	"mftb  %1 \n\t"		\
	"mftbu %2 \n\t"		\
	: "=r"(((long*)&val)[0]), "=r"(((long*)&val)[1]), "=r"(tmp)	\
	:			\
	);			\
    } while (tmp != (((long*)&val)[0]));	\
}

#elif defined(__i386__)

#define READ_TIMESTAMP(val) \
    __asm__ __volatile__("rdtsc" : "=A" (val))

#elif defined(__x86_64__)

#define READ_TIMESTAMP(val) \
    __asm__ __volatile__("rdtsc" : "=a" (((int*)&val)[0]), "=d" (((int*)&val)[1]));

#else

#define READ_TIMESTAMP(var) readTSC(&var)

static void readTSC(volatile uint64_t *v)
{
    struct timespec tp;
    clock_gettime (CLOCK_REALTIME, &tp);
    *v = (uint64_t)(tp.tv_sec * (uint64_t)1000000000) + (uint64_t)tp.tv_nsec;
}

#endif

static char *romName[] = {
    "str$08.roz",
    "str$09.roz",
    "str$0a.roz",
    "str$0b.roz",
    "str$0c.roz",
    "str$0d.roz",
    "str$0e.roz",
    "str$0f.roz"
};

static char *romDiskName[] = {
    "rom0.roz",
    "rom1.roz",
    "rom2.roz",
    "rom3.roz",
    "rom4.roz"
};

static int LoadRom(char *name, int p)
{
    char ftemp[256];
    char namp[9];
    namp[8] = 0;

    sprintf(ftemp, "%s/Rom/%s", datadir, name);

    gzFile fi = gzopen(ftemp, "rb");
    if (fi) {
	while ( gzread(fi, mc6800_getRomPtr(0, p), 8192) == 8192 && p < 16) {
	    memcpy(namp, mc6800_getRomPtr(0, p)+2, 8);
	    fprintf(stderr, "%s as page %d\n", namp, p);
	}
	gzclose(fi);
    }
    p++;
    return p;
}

static int LoadRomDisk(char *name, int c)
{
    char ftemp[256];
    char namp[9];
    namp[8] = 0;

    sprintf(ftemp, "%s/Rom/%s", datadir, name);

    gzFile fi = gzopen(ftemp, "rb");
    if (fi) {
	int size = gzread(fi, mc6800_getRomPtr(c, 0), 65536);
	if ((size > 0) & (!(size & 0x1fff))) {
	    fprintf(stderr, "%s as ROM %d : 27%d\n", name, c, size / 1024 * 8);
	    int ptr = size;
	    while (ptr < 65536) {
		memcpy(mc6800_getRomPtr(c, 0) + ptr, mc6800_getRomPtr(c, 0), size);
		ptr += size;
	    }
	}
	gzclose(fi);
    }
    c++;
    return c;
}

static void ChecKeyboard(void)
{
    SDL_Event event;
    int x = 0, y = 0;
    int vmenu_process = 0;

    if(SDL_WaitEvent(&event) > 0){
	switch(event.type) {
	    case SDL_QUIT:
		exitRequested = 1;
		break;
#ifdef USE_JOYSTICK
	    case SDL_JOYBUTTONDOWN:
		switch(event.jbutton.button) {
		    case JOYBUT_HOME: 	exitRequested = 1; break; //OFF
		    case JOYBUT_START: 	resetRequested = 1; break;//RESET
		    case JOYBUT_UP:
			if (filemenuEnabled) {
			    FloppyManagerUpdateList(-1);
			} else
			    jkeybDown(0x48); 
			break;
		    case JOYBUT_DOWN:
			if (filemenuEnabled) {
			    FloppyManagerUpdateList(1);
			} else
			    jkeybDown(0x50); 
			break;
		    case JOYBUT_LEFT:	jkeybDown(0x4b); break;
		    case JOYBUT_RIGHT:	jkeybDown(0x4d); break;
		    case JOYBUT_TRIANGLE: jkeybDown(0x01); break; //ESC
		    case JOYBUT_CROSS:
			if (filemenuEnabled) {
			    selectFloppyByNum();
			    FloppyManagerOff();
			    redrawVMenu = 1;
			    clearVScr = 1;
			    filemenuEnabled = 0;
#ifdef USE_JOYMOUSE
			} else if (vkbdEnabled) {
			    x = joymouse_x;
			    y = joymouse_y;
			    vmenu_process = 1;
#endif
			} else
			    jkeybDown(0x39);   //SPACE 
			break;
		    case JOYBUT_SQUARE:	jkeybDown(0x0f); break;   //TAB
		    case JOYBUT_CIRCLE:	jkeybDown(0x1c); break;   //RETURN
#ifdef USE_JOYMOUSE
		    case JOYBUT_RTRIGGER:
			if (filemenuEnabled)
			    break;
			vkbdEnabled = vkbdEnabled?0:1;
			redrawVMenu = 1;
			clearVScr = 1;
			break;
#endif
		    case JOYBUT_LTRIGGER:
			if (vkbdEnabled)
			    break;
			if (!filemenuEnabled) {
			    filemenuEnabled = 1;
			    FloppyManagerOn(FLOPPY_A, datadir);
			} else {
			    FloppyManagerOff();
			    redrawVMenu = 1;
			    clearVScr = 1;
			    filemenuEnabled = 0;
			}
			break;
		    case JOYBUT_SELECT:	savepng(vscr, 320 * vScale, 27 * 8 * vScale); break;
		    default: 		jkeybDown(0x39);
		}
		break;
	    case SDL_JOYBUTTONUP:
		switch(event.jbutton.button) {
		    case JOYBUT_SELECT:
		    case JOYBUT_LTRIGGER:
#ifdef USE_JOYMOUSE
		    case JOYBUT_RTRIGGER:
#endif
			break;
		    default:		jkeybUp();
		}
		break;
#ifdef USE_JOYMOUSE
	    case SDL_JOYAXISMOTION:
		//SDL_ShowCursor(1);
		//SDL_WarpMouse(160, 100);
		joymouse_enabled = 1;
		//joymouse_x++;
		//joymouse_y++;
		if (event.jaxis.axis == 0) {
		    int x = event.jaxis.value + 32768;
		    x *= 320 * vScale;
		    x /= 65535;
		    joymouse_x += (x - 160) / 10;
		    if (joymouse_x < 0)
			joymouse_x = 0;
		    else if (joymouse_x > (320 - 8))
			joymouse_x = 320 - 8;
		} else if (event.jaxis.axis == 1) {
		    int y = event.jaxis.value + 32768;
		    y *= 240 * vScale;
		    y /= 65535;
		    joymouse_y += (y - 120) / 10;
		    if (joymouse_y < 0)
			joymouse_y = 0;
		    else if (joymouse_y > (240 - 8))
			joymouse_y = 240 - 8;
		}
		break;
#endif
#endif
	    case SDL_KEYDOWN: {
		SDLKey sdlkey=event.key.keysym.sym;
		int k=0;
		switch(sdlkey){
		case SDLK_UP:		jkeybDown(0x48); break;
		case SDLK_DOWN:		jkeybDown(0x50); break;
		case SDLK_LEFT:		jkeybDown(0x4b); break;
		case SDLK_RIGHT:	jkeybDown(0x4d); break;
		case SDLK_ESCAPE:	jkeybDown(0x01); break;
		
		case SDLK_F1:		jkeybDown(0x3b); break;
		case SDLK_F2:		jkeybDown(0x3c); break;
		case SDLK_F3:		jkeybDown(0x3d); break;
		case SDLK_F4:		jkeybDown(0x3e); break;
		case SDLK_F5:		jkeybDown(0x3f); break;
		case SDLK_F6:		jkeybDown(0x40); break;
		case SDLK_F7:		jkeybDown(0x41); break;
		case SDLK_F8:		jkeybDown(0x42); break;
		case SDLK_F9:		jkeybDown(0x43); break;
		case SDLK_F10:		jkeybDown(0x44); break;
		case SDLK_F11:		jkeybDown(0x57); break;
		case SDLK_F12:		jkeybDown(0x58); break;
		
		case SDLK_HOME:		jkeybDown(0x47); break;
		case SDLK_END:		jkeybDown(0x4f); break;
		case SDLK_INSERT:	jkeybDown(0x52); break;
		
		case SDLK_1:		jkeybDown(0x02); break;
		case SDLK_2:		jkeybDown(0x03); break;
		case SDLK_3:		jkeybDown(0x04); break;
		case SDLK_4:		jkeybDown(0x05); break;
		case SDLK_5:		jkeybDown(0x06); break;
		case SDLK_6:		jkeybDown(0x07); break;
		case SDLK_7:		jkeybDown(0x08); break;
		case SDLK_8:		jkeybDown(0x09); break;
		case SDLK_9:		jkeybDown(0x0a); break;
		case SDLK_0:		jkeybDown(0x0b); break;
		case SDLK_MINUS:	jkeybDown(0x0c); break;
		case SDLK_EQUALS:	jkeybDown(0x0d); break;
		case xSDLK_BACKSLASH:	jkeybDown(0x2b); break;
		case SDLK_BACKSPACE:	jkeybDown(0x0e); break;
		case xSDLK_LSUPER:	jkeybDown(0x46); break;

		case SDLK_TAB:		jkeybDown(0x0f); break;
		case xSDLK_q:		jkeybDown(0x10); break;
		case xSDLK_w:		jkeybDown(0x11); break;
		case xSDLK_e:		jkeybDown(0x12); break;
		case xSDLK_r:		jkeybDown(0x13); break;
		case xSDLK_t:		jkeybDown(0x14); break;
		case xSDLK_y:		jkeybDown(0x15); break;
		case xSDLK_u:		jkeybDown(0x16); break;
		case xSDLK_i:		jkeybDown(0x17); break;
		case xSDLK_o:		jkeybDown(0x18); break;
		case xSDLK_p:		jkeybDown(0x19); break;
		case xSDLK_BACKQUOTE:	jkeybDown(0x29); break;
		case SDLK_RETURN:	jkeybDown(0x1c); break;

		case xSDLK_a:		jkeybDown(0x1e); break;
		case xSDLK_s:		jkeybDown(0x1f); break;
		case xSDLK_d:		jkeybDown(0x20); break;
		case xSDLK_f:		jkeybDown(0x21); break;
		case xSDLK_g:		jkeybDown(0x22); break;
		case xSDLK_h:		jkeybDown(0x23); break;
		case xSDLK_j:		jkeybDown(0x24); break;
		case xSDLK_k:		jkeybDown(0x25); break;
		case xSDLK_l:		jkeybDown(0x26); break;
		case xSDLK_SEMICOLON:	jkeybDown(0x27); break;
		case xSDLK_QUOTE:	jkeybDown(0x28); break;
		case xSDLK_LEFTBRACKET:	jkeybDown(0x1a); break;
		case xSDLK_RIGHTBRACKET:	jkeybDown(0x1b); break;

		case xSDLK_z:		jkeybDown(0x2c); break;
		case xSDLK_x:		jkeybDown(0x2d); break;
		case xSDLK_c:		jkeybDown(0x2e); break;
		case xSDLK_v:		jkeybDown(0x2f); break;
		case xSDLK_b:		jkeybDown(0x30); break;
		case xSDLK_n:		jkeybDown(0x31); break;
		case xSDLK_m:		jkeybDown(0x32); break;
		case xSDLK_COMMA:	jkeybDown(0x33); break;
		case xSDLK_PERIOD:	jkeybDown(0x34); break;
		case SDLK_SLASH:	jkeybDown(0x35); break;

		case SDLK_SPACE:	jkeybDown(0x39); break;

		case SDLK_CAPSLOCK:	jkeybDown(0x3a); break;
		
		case SDLK_LCTRL:	flagKey|=1; break;
		case SDLK_LSHIFT:	flagKey|=2; break;
		case SDLK_RCTRL:	flagKey|=1; break;
		case SDLK_RSHIFT:	flagKey|=2; break;

		case SDLK_PRINT:	savepng(vscr, 320 * vScale, 27 * 8 * vScale); break;
		case SDLK_PAUSE:	resetRequested = 1; break;
		case SDLK_SCROLLOCK:
		    if (!SDL_WM_ToggleFullScreen(screen)) {
			fprintf(stderr, "Unable change video mode!\n");
		    }
		    break;
		
		default:
		    k = (int)sdlkey;
		    fprintf(stderr, "key=%d\n", sdlkey);
		}
//		key(k, 0);
		}; break;
	    case SDL_KEYUP: {
		SDLKey sdlkey=event.key.keysym.sym;
		switch(sdlkey){

		case SDLK_LCTRL:	flagKey&=~1; break;
		case SDLK_LSHIFT:	flagKey&=~2; break;
		case SDLK_RCTRL:	flagKey&=~1; break;
		case SDLK_RSHIFT:	flagKey&=~2; break;

		default:
		    jkeybUp();
		}
		}; break;
	    case SDL_MOUSEBUTTONDOWN: {
	    	x = (event.button.x - ((vscr_width - 320 * vScale) >> 1)) / vScale;
		y = (event.button.y - ((vscr_height - 240 * vScale) >> 1)) / vScale;
		vmenu_process = 1;
		}; break;
	    case SDL_MOUSEBUTTONUP:
		if (vkbdEnabled) vkeybUp();
	    default:
		break;
	}
	
	if (vmenu_process) {
	    if (filemenuEnabled) {
		selectFloppy(y);
		FloppyManagerOff();
		redrawVMenu = 1;
		clearVScr = 1;
		filemenuEnabled = 0;
	    } else {
		if (vkbdEnabled) vkeybDown(x, y);
		if (y > 216) {
		    if (x > 5 && x < 21) {
			//power off
			exitRequested=1;
		    } else if (x > 33 && x < 50 && vkbdEnabled == 0) {
			filemenuEnabled = 1;
			FloppyManagerOn(FLOPPY_A, datadir);
		    } else if (x > 63 && x < 80 && vkbdEnabled == 0) {
			filemenuEnabled = 1;
			FloppyManagerOn(FLOPPY_B, datadir);
		    } else if (x > 93 && x < 110) {
			if (!SDL_WM_ToggleFullScreen(screen)) {
			    fprintf(stderr, "Unable change video mode!\n");
			}
		    } else if (x > 123 && x < 140) {
			savepng(vscr, 320 * vScale, 27 * 8 * vScale);
		    } else if (x > 274 && x < 300) {
			vkbdEnabled = vkbdEnabled?0:1;
			//if (vkbdEnabled == 0) 
			redrawVMenu = 1;
			clearVScr = 1;
		    }
		}
	    }
	}
    }
}

int SDLCALL HandleKeyboard(void *unused)
{
    while (!exitRequested)
	ChecKeyboard();
    return 0;
}

int SDLCALL HandleVideo(void *unused)
{
    while (!exitRequested) {
#ifndef __APPLE__
	SDL_Flip( screen );
#endif
	if ( ! filemenuEnabled ) {
	    refreshScr();
	}

	if (show_info) {
	    char buf[64];

	    sprintf(buf, "%1.2fMHz", (float)actual_speed / 1000);
	    drawString(160, 28, buf, 0xffff, 0);
	}

#ifdef USE_JOYSTICK
#ifdef USE_JOYMOUSE
	if (joymouse_enabled && (vkbdEnabled || (joymouse_y >= 216))) {
	    drawChar(joymouse_x, joymouse_y, '+', 0xff00, 0);
	    if (joymouse_y >= 216) 
		redrawVMenu = 1;
	}
#endif
#endif
	while(!(updateScreen || exitRequested))
	    usleep(1000);

	updateScreen = 0;
    }
    return 0;
}

#ifdef PYLDIN_LOGO
#include "logo.h"
#include <SDL_image.h>

#ifdef __MINGW32__
#define	sleep(a)	_sleep((unsigned long)(a) * 1000l);
#endif

SDL_Surface *pyldin_logo;

void LoadLogo(void)
{
    fprintf(stderr, "Loading logo... ");

    SDL_RWops *src = SDL_RWFromMem(pyldin_foto, pyldin_foto_len);

    SDL_Surface *tmp = IMG_LoadJPG_RW(src);

    if (!tmp)
	fprintf(stderr, "Failed!\r\n");
    else {
	pyldin_logo = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					tmp->clip_rect.w, 
					tmp->clip_rect.h, 
					screen->format->BitsPerPixel, 
					screen->format->Rmask,
					screen->format->Gmask,
					screen->format->Bmask,
					screen->format->Amask
					);

	SDL_BlitSurface(tmp, NULL, pyldin_logo, NULL);

	SDL_FreeSurface(tmp);
	fprintf(stderr, "Ok!\r\n");
    }
    SDL_FreeRW(src);
}

#endif

#ifdef PYLDIN_ICON

#include "icon.h"

void SetIcon(void)
{
    SDL_Surface *icon;

    fprintf(stderr, "Loading icon... ");

    SDL_RWops *src = SDL_RWFromMem(pyldin_icon, pyldin_icon_len);

    icon = IMG_LoadPNG_RW(src);

    if (!icon) {
	fprintf(stderr, "Failed!\r\n");
	return;
    } else
	fprintf(stderr, "Ok!\r\n");

    SDL_FreeRW(src);

    SDL_WM_SetIcon(icon, NULL);

    SDL_FreeSurface(icon);
}
#endif

void usage(char *app)
{
    fprintf(stderr, "Usage: %s [-d <dir>][-h][-i][-t][-p <type>][-s <N>][boot floppy image]\n", app);
    fprintf(stderr, "-d <dir>  - path to directory with Rom/Floppy images\n");
    fprintf(stderr, "-g WxH    - set screen geometry WxH\n");
    fprintf(stderr, "-h        - this help\n");
    fprintf(stderr, "-i        - show cpu performance\n");
    fprintf(stderr, "-t        - setup date&time from host\n");
    fprintf(stderr, "-p <type> - function of printer port:\n");
    fprintf(stderr, "            file   - output to file\n");
    fprintf(stderr, "            system - output to default system printer\n");
    fprintf(stderr, "            covox  - output to unsigned 8bit DAC (COVOX emulation)\n");
    fprintf(stderr, "-s <N>    - scale screen x N\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int set_time = 0;
    int printer_type = PRINTER_NONE;
    char *bootFloppy = NULL;
    Uint32 vid_flags;
    Uint32 sys_flags;
#ifdef USE_JOYSTICK
    SDL_Joystick *joystick;
#endif
    SDL_Thread *video_thread;
    SDL_Thread *keybd_thread;

    fprintf(stderr, "Portable Pyldin-601 emulator version " VERSION " (http://code.google.com/p/pyldin)\n");
    fprintf(stderr, "Copyright (c) 1997-2009 Sasha Chukov <sash@pdaXrom.org>, Yura Kuznetsov <yura@petrsu.ru>\n");

    extern char *optarg;
    extern int optind, optopt, opterr;
    int c;

    while ((c = getopt(argc, argv, "hits:d:p:g:")) != -1)
    {
        switch (c)
        {
	case 'h':
	    usage(argv[0]);
	    break;
	case 's':
	    vScale = atoi(optarg);
	    fprintf(stderr, "scale %d\n", vScale);
	    break;
	case 't':
	    set_time = 1;
	    break;
	case 'i':
	    show_info = 1;
	    break;
        case 'd':
	    datadir = optarg;
            break;
	case 'p':
	    if (!strcmp(optarg, "file"))
		printer_type = PRINTER_FILE;
	    else if (!strcmp(optarg, "covox"))
		printer_type = PRINTER_COVOX;
	    else if (!strcmp(optarg, "system"))
		printer_type = PRINTER_SYSTEM;
	    break;
	case 'g':
	    sscanf(optarg, "%dx%d", &vscr_width, &vscr_height);
	    break;
        default:
	    usage(argv[0]);
            break;
        }
    }

    for ( ; optind < argc; optind++)
	bootFloppy = argv[optind];

    sys_flags = 0;
#ifdef USE_JOYSTICK
    sys_flags |= SDL_INIT_JOYSTICK;
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | sys_flags) < 0) {
	fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
	exit(1);
    }

#ifdef USE_JOYSTICK
    joystick = SDL_JoystickOpen(0);

    if (joystick == NULL)
	fprintf(stderr, "No joystick detected\n");
    else
	fprintf(stderr, "Use joystick %s\n", SDL_JoystickName(SDL_JoystickIndex(joystick)));		
#endif

    vscr_width = (vscr_width < 320)?320:vscr_width;
    vscr_height = (vscr_height < 240)?240:vscr_height;

    while (((vscr_width < (320 * vScale)) || (vscr_height < (240 * vScale))) && (vScale > 1))
	vScale--;

    fprintf(stderr, "Set screen geometry to %dx%d, scale %d\n", vscr_width, vscr_height, vScale);

#ifdef PYLDIN_ICON
    SetIcon();
#endif

    SDL_WM_SetCaption("PYLDIN-601 Emulator v" VERSION, "pyldin");

    vid_flags = SDL_HWSURFACE;
    vid_flags |= SDL_DOUBLEBUF;

    screen = SDL_SetVideoMode(vscr_width, vscr_height, 16, vid_flags);

    char ftemp[256];

#ifdef PYLDIN_LOGO
    LoadLogo();
    
    if (pyldin_logo) {
	SDL_SoftStretch(pyldin_logo, &pyldin_logo->clip_rect, screen, &screen->clip_rect);
	SDL_Flip(screen);
	sleep(1);
    }
#endif

    mc6800_init();

    fprintf(stderr, "Loading main rom... ");
    sprintf(ftemp, "%s/Bios/bios.roz", datadir);

    gzFile fi = gzopen(ftemp, "rb");

    if (fi) {
	byte *m = mc6800_getRomPtr(-1, -1); 
	gzread(fi, m, 4096);
	gzclose(fi); 
	fprintf(stderr, "Ok\r\n");
    } else { 
	fprintf(stderr, "Failed!\r\n"); 
	return -1; 
    }

    fprintf(stderr, "Loading font rom... ");
    sprintf(ftemp, "%s/Bios/video.roz", datadir);

    if (loadTextFont(ftemp) == 0) 
	fprintf(stderr, "Ok\r\n");
    else { 
	fprintf(stderr, "Failed!\r\n"); 
	return -1; 
    }

    fprintf(stderr, "Detecting host cpu speed... ");
    {
	volatile uint64_t a;
	READ_TIMESTAMP(a);
	sleep(1);
	READ_TIMESTAMP(one_takt_delay);
#ifdef __PPC__
#warning "PPC PS3 calculation, fixme"
	one_takt_delay -= a;
	fprintf(stderr, "%lld MHz\n", one_takt_delay * 4 / 100000);
	one_takt_delay /= 1000000;
	one_takt_calib = one_takt_delay;
#else
	one_takt_delay -= a;
	one_takt_delay /= 1000000;
	one_takt_calib = one_takt_delay;
	fprintf(stderr, "%lld MHz\n", one_takt_calib);
#endif
    }

    int j = 0;
    int i;

    for (i = 0; i < 8; i++)
	j = LoadRom(romName[i], j);

    for (i = 0; i < 5; i++)
	LoadRomDisk(romDiskName[i], i);

    initFloppy();

    if (bootFloppy)
	insertFloppy(FLOPPY_A, bootFloppy);

    printer_init(printer_type);

    // sound initialization
    Speaker_Init();

    updateScreen = 0;
    video_thread = SDL_CreateThread(HandleVideo, NULL);
    keybd_thread = SDL_CreateThread(HandleKeyboard, NULL);

    mc6800_reset();

    vkbdEnabled = 0;
    redrawVMenu = 1;
    exitRequested = 0;
    IRQrequest = 0;
    filemenuEnabled = 0;

    int vcounter = 0;		//
    int scounter = 0;		// syncro counter
    int takt;

    volatile uint64_t clock_old;
    READ_TIMESTAMP(clock_old);

    vscr = (unsigned short *) screen->pixels;

    if (set_time) {
	struct tm *dt;
	time_t t = time(NULL);
	dt = localtime(&t);
	mc6800_setDATETIME( dt->tm_year,
			    dt->tm_mon,
			    dt->tm_mday,
			    dt->tm_hour,
			    dt->tm_min,
			    dt->tm_sec
			    );
    }

    volatile uint64_t ts1;
    READ_TIMESTAMP(ts1);
    do {
	takt = mc6800_step();

	vcounter += takt;
	scounter += takt;

	if (vcounter >= 20000) {
#ifdef __APPLE__
	    SDL_Flip( screen );
#endif
	    tick50 = 0x80;
	    curBlink++;
	    IRQrequest = 1;
	    updateScreen = 1;

	    volatile uint64_t clock_new;
	    READ_TIMESTAMP(clock_new);
	    actual_speed = (vcounter * 1000) / ((clock_new - clock_old) / one_takt_calib);
	
	    clock_old = clock_new;
	    vcounter = 0;
	}
	if (resetRequested == 1) {
	    mc6800_reset();
	    resetRequested = 0;
	}

	volatile uint64_t ts2;
	do {
	    READ_TIMESTAMP(ts2);
	} while ((ts2 - ts1) < (one_takt_delay * takt));
	ts1 = ts2;
    } while( exitRequested == 0);	//

    freeFloppy();

    mc6800_fini();
    Speaker_Finish();
    printer_fini();

#ifdef USE_JOYSTICK
    if (joystick)
	SDL_JoystickClose(joystick);
#endif

#ifdef PYLDIN_LOGO
    if (pyldin_logo) {
	SDL_SoftStretch(pyldin_logo, &pyldin_logo->clip_rect, screen, &screen->clip_rect);
	SDL_Flip(screen);
	sleep(1);
    }
#endif
	
//    SDL_Quit();
    return 0;
}

