/*
 *
 * Pyldin-601 emulator version 3.1 for Linux,MSDOS,Win32
 * Copyright (c) Sasha Chukov & Yura Kuznetsov, 2000-2004
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "wave.h"

#define FREQ	44100

static short *buffer;
static int buffer_size;
static int buffer_pos;

static int old_ticks;
static int old_val;

static int fInited = 0;

static snd_pcm_t *handle;

int Speaker_Init(void)
{
    int rc, i;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir = 0;
    snd_pcm_uframes_t frames;

    char *audiodev = getenv("AUDIODEV");

    /* Open PCM device for playback. */
    rc = snd_pcm_open(&handle, audiodev?audiodev:"default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (rc < 0) {
	fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
	return -1;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

#if defined(__PPC__)
    /* Signed 16-bit big-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_BE);
#else
    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
#endif

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = FREQ;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Set period size to 32 frames. */
    frames = 256;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
	fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
	return -1;
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    buffer_size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (short *) malloc (buffer_size);
    for(i = 0 ; i < (buffer_size >> 1); i++ )
	buffer[i] = 0;

    buffer_pos = 0;
    fInited = 1;
    old_val = 0;
    old_ticks = 0;

    return 0;
}

void Speaker_Finish(void)
{
    if (fInited) {
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
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

    unsigned int slen = ((ticks - old_ticks) * buffer_size) / 20000;
    int loops = 0;
    if (slen > 0) {
	while (slen-- > 0) {
	    buffer[buffer_pos++] = (old_val << 8) ^ 0x8000;
	    buffer[buffer_pos++] = (old_val << 8) ^ 0x8000;
	    if (buffer_pos >= (buffer_size >> 1)) {
		int rc = snd_pcm_writei(handle, buffer, buffer_pos >> 1);
		if (rc == -EPIPE) {
		    /* EPIPE means underrun */
		    //fprintf(stderr, "underrun occurred\n");
		    snd_pcm_prepare(handle);
		} else if (rc < 0) {
		    fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
		} else if (rc != (buffer_size >> 2)) {
		    fprintf(stderr, "short write, write %d frames\n", rc);
		}
	        buffer_pos = 0;
		if (loops++ > 1)
		    break;
	    }
	}

    }
    old_ticks = ticks;
    old_val = val;
}
