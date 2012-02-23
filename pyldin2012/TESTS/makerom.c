#include <stdio.h>

void int2bin(int n)
{
    int i;
    for (i = 0; i < 8; i++) {
	printf("%c", (n&0x80)?'1':'0');
	n <<= 1;
    }
}

int main(int argc, char *argv[])
{
    FILE *f = fopen(argv[1], "rb");
    if (f) {
	unsigned char c;
	int i = 0;
	fseek(f, 0, SEEK_END);
	long l = ftell(f);
	fseek(f, 0, SEEK_SET);
	while (fread(&c, 1, 1, f) > 0) {
//	    printf("%04d => x\"%02x\", ", i, c);
	    printf("x\"%02x\", ", c);
	    i++;
	    if ((i % 16) == 0)
		printf("\n");
	}
	fclose(f);
    }
    return 0;
}
