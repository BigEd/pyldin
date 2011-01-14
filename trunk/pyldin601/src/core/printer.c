#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "core/mc6800.h"
#include "core/printer.h"

static int p_mode = PRINTER_NONE;

static unsigned char port_dra = 0;
static unsigned char port_drb = 0;
static unsigned char port_cra = 0;
static unsigned char port_crb = 0;

int printer_init(int mode)
{
    if (mode == PRINTER_SYSTEM) {
	mode = PRINTER_NONE;
    }
    p_mode = mode;

    return 0;
}

int printer_fini(void)
{
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
	printer_put_char(data);
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
