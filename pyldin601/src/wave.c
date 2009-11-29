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

#define FREQ	48000

static short *buffer;
static int buffer_size;
static int buffer_pos;

static int old_ticks;
static int old_val;

static int fInited = 0;
static int fEnabled = 0;

static void callback(void *data, Uint8 *stream, int len)
{
    int i;
    int pos = 0;
    SDL_AudioSpec *obt = (SDL_AudioSpec *) data;
    short *ptr = (short *) stream;

    for (i = 0; i < len/4; i++) {
	ptr[i*2] = buffer[pos];
	ptr[i*2+1] = buffer[pos];
	buffer[pos++] = 0;
	if (pos >= obt->samples)
	    break;
    }

    buffer_pos = 0;
}

int Speaker_Init(void)
{
    int i;
    SDL_AudioSpec wav, obt;

    fInited = 0;
    fEnabled = 0;

    wav.freq = FREQ;			/* Frequency */
    wav.format = AUDIO_S16;		/* Format */
    wav.channels = 2;			/* 1 = mono, 2 = stereo */
    wav.samples = wav.freq / 50;	/* Good low-latency value for callback */
    wav.callback = callback;
    wav.userdata = &obt;

    if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 )
    {
	fprintf(stderr, "Couldn't init audio: %s\n", SDL_GetError());
	return -1;
    }

    if ( SDL_OpenAudio(&wav, &obt) < 0 ) {
	fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	return -1;
    }

    buffer_size = obt.samples;
    buffer = (short *) malloc (obt.samples * 2);
    for(i = 0 ; i < obt.samples; i++ )
	buffer[i] = 0;

    buffer_pos = 0;
    fInited = 1;
    old_val = 0;
    old_ticks = 0;

    SDL_PauseAudio(0);

    return 0;
}

void Speaker_Finish(void)
{
    if (fInited) {
	SDL_PauseAudio(1);
	if (buffer) 
	    free(buffer);
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

    SDL_LockAudio();
    unsigned int slen = ((ticks - old_ticks) * buffer_size) / 20000;
    if ((buffer_pos < buffer_size) &&
	(slen > 0)) {

	while (slen-- > 0) {
	    buffer[buffer_pos++] = (old_val << 8) ^ 0x8000;
	    if (buffer_pos >= buffer_size)
	        break;
	}

    }
    old_ticks = ticks;
    old_val = val;
    SDL_UnlockAudio();
}
