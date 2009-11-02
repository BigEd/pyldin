/*
 *
 * mc6800 emulator version 2.0 for BeOS / Linix / Win9x/NT
 * Copyright (c) Sasha Chukov (sash@pdaXrom.org), 1997-2006
 *
 */
#include <stdio.h>

char romp[65536];

int chips[] = {
    65536,
    32768,
    32768,
    32768
};

int main(int argc, char *argv[])
{
	FILE *fo, *fi;
	char namp[9];
	char namef[81];
	int p = 0;
	int size;

	printf(	"SplitDisk utility v1.0 for Pyldin-601 emulator\n"
		"copyright (c) Sasha Chukov (sash@pdaXrom.org), 2000-2006\n");

	if (argc < 2) {
	    fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
	    return -1;
	}

	namp[8] = 0;

	fi = fopen(argv[1], "rb");
	if (fi) {
	  while ( (size = fread(romp, 1, chips[p], fi)) > 0) {
	    sprintf(namef, "rom%d.rom", p + 1);
	    fo = fopen(namef, "wb");
	    fwrite(romp, 1, size, fo);
	    fclose(fo);
	    p++;
	  }
	  fclose(fi);
	} else {
	  fprintf(stderr, "%s - Can't open file!\n", argv[1]);    
	}
	
	return 0;
}
