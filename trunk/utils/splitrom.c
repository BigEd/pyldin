/*
 *
 * mc6800 emulator version 2.0 for BeOS / Linix / Win9x/NT
 * Copyright (c) Sasha Chukov (sash@pdaXrom.org), 1997-2006
 *
 */
#include <stdio.h>
#include <string.h>

char romp[8192];

int main(int argc, char *argv[])
{
	FILE *fo, *fi;
	char namp[9];
	char namef[81];
	int p = 0;

	printf(	"SplitRom utility v1.0 for Pyldin-601 emulator\n"
		"copyright (c) Sasha Chukov (sash@pdaXrom.org), 2000-2006\n");

	if (argc < 2) {
	    fprintf(stderr, "Usage: %s <rom image>\n", argv[0]);
	    return -1;
	}

	namp[8] = 0;

	fi = fopen(argv[1], "rb");
	if (fi) {
	  while ( fread(romp, 1, 8192, fi) == 8192) {
	    memcpy(namp, romp+2, 8);
	    printf("Loaded %s\n", namp);
	    sprintf(namef, "str$%02x.rom", p + 8);
	    p++;
	    fo = fopen(namef, "wb");
	    fwrite(romp, 1, 8192, fo);
	    fclose(fo);
	  }
	  fclose(fi);
	} else {
	  fprintf(stderr, "%s - Can't open file!\n", argv[1]);    
	}
	
	return 0;
}
