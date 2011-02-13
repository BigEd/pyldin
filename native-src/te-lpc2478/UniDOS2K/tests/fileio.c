#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *f = fopen(argv[1], "r");
    if (f) {
	while (!feof(f)) {
#if 0
	    char buf[512];
	    memset(buf, 0, 512);
	    if (fgets(buf, 512, f)) {
		printf(">%s\n", buf);
	    } else
		fprintf(stderr, "no more data!\n");
#else
	    char c = getc(f);
	    putchar(c);
#endif
	}
	fclose(f);
    } else
	fprintf(stderr, "Can't open file %s\n", argv[1]);
    return 0;
}
