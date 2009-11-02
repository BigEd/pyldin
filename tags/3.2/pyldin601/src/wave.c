/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include "wave.h"

#define FREQ	22050
#define BUFSIZE	FREQ
//FREQ/20

static short *buffer;

static int pos;
static int pos_count;
static int old_ticks;
static int old_val;

static int posp;
static int posp_count;

static int fInited = 0;

static void callback(void *unused, Uint8 *stream, int len)
{
    int i;
    short *ptr = (short *) stream;
    for (i=0; i<len/2; i++) {
	if (pos_count > posp_count) {
	    ptr[i] = buffer[posp];
	    buffer[posp++] = 0;
	    posp_count++;
	} else {
	    ptr[i] = 0;
	}
	if (posp >= BUFSIZE) 
	    posp = 0;
    }
    pos_count -= posp_count;
    posp_count = 0;
}

int Speaker_Init(void)
{
    int i;
    SDL_AudioSpec wav, obt;

    fInited = 0;

    wav.freq = FREQ;		//mode;
    wav.format = AUDIO_S16;
    wav.channels = 1;		/* 1 = mono, 2 = stereo */
    wav.samples = 64;		/* Good low-latency value for callback */
    wav.callback = callback;
    wav.userdata = NULL;

    if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 )
    {
	fprintf(stderr, "Couldn't init audio: %s\n", SDL_GetError());
	return -1;
    }

    if ( SDL_OpenAudio(&wav, &obt) < 0 ) {
	fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	return -1;
    }

    buffer = (short *) malloc (BUFSIZE * 2);
    for(i = 0 ; i < BUFSIZE; i++ )
	buffer[i] = 0; //(i & 8)?0xff:0x0;;

    pos = 0;
    posp = 0;
    old_ticks = 0;
    old_val = 0;
    pos_count = 0;
    posp_count = 0;

    SDL_PauseAudio(0);

    fInited = 1;

    return 0;
}

void Speaker_Finish(void)
{
    if (fInited) {
	SDL_PauseAudio(1);
	if (buffer) 
	    free(buffer);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	fInited = 0;
    }
}

void Speaker_Set(int val, int ticks)
{
    Covox_Set(val?0xff:0, ticks);
}

void Covox_Set(int val, int ticks)
{
    if (!fInited)
	return;
    int slen = ((ticks - old_ticks)*1000)/FREQ; //40000; //20000;
    if (slen > BUFSIZE) slen = BUFSIZE;

//    SDL_PauseAudio(1);

    SDL_LockAudio();

    while (slen > 0) {
	buffer[pos++] = (old_val << 8) ^ 0x8000;
	if (pos >= BUFSIZE) 
	    pos = 0;
	slen--;
	pos_count++;
    }

    SDL_UnlockAudio();

    old_ticks = ticks;
    old_val = val;
}

void Speaker_Clear(int ticks)
{
    if (!fInited)
	return;
//    SDL_PauseAudio(0);
/*
	int slen = ((ticks - old_ticks)*BUFSIZE)/40000; //20000;
	if (slen > BUFSIZE) slen = BUFSIZE;

	while (slen > 0) {
		buffer[pos++] = (old_val)?0x7fff:0;
		if (pos >= BUFSIZE) pos = 0;
		slen--;
	}

	old_ticks = ticks;
*/
}
