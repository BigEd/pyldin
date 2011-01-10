#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include "mc6800.h"
#include "wave.h"
#include "printer.h"

#define PRNFILE	"pyldin.prn"

static FILE *prn = NULL;
static int p_mode = PRINTER_NONE;

unsigned char port_dra;
unsigned char port_drb;
unsigned char port_cra;
unsigned char port_crb;

int printer_init(int mode)
{
    if (mode == PRINTER_SYSTEM) {
	fprintf(stderr, "System printer unsupported yet!\n");
	mode = PRINTER_NONE;
    }
    p_mode = mode;
    
    port_dra = 0;
    port_drb = 0;
    port_cra = 0;
    port_crb = 0;
    
    return 0;
}

int printer_fini(void)
{
    if ((p_mode == PRINTER_FILE) && prn)
	fclose(prn);

    return 0;
}

int printer_dra_wr(unsigned char data)
{
    port_dra = data;

    return 0;
}

int printer_drb_wr(unsigned char data)
{
    if (p_mode == PRINTER_FILE) {
	if (!prn) {
	    char file[256];
	    char *home = getenv("HOME");
	    if (home)
		sprintf(file, "%s/%s", home, PRNFILE);
	    else
		strcpy(file, PRNFILE);
		
	    fprintf(stderr, "Printer file: %s\n", file);

	    prn = fopen(file, "wb");
	}
	if (prn)
	    fputc(data, prn);
    } else if (p_mode == PRINTER_COVOX)
	Covox_Set(data, mc6800_get_takts());

    port_drb = data;
    
    return 0;
}

int printer_cra_wr(unsigned char data)
{
    port_cra = data;

    return 0;
}

int printer_crb_wr(unsigned char data)
{
    port_crb = data;

    return 0;
}

unsigned char printer_dra_rd(void)
{
    if (p_mode == PRINTER_FILE)
	return 0x0;

    return port_dra;
}

unsigned char printer_drb_rd(void)
{
    return port_drb;
}

unsigned char printer_cra_rd(void)
{
    return port_cra;
}

unsigned char printer_crb_rd(void)
{
    return port_crb;
}
