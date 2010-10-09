#include <stdio.h>
#define INI_REG 2

char line[80];
int cnt;
int scount = 0, dcount = 0;

pack (char c)
{
	static int chars[3] = {0,0,0};
	static cnt = 2;
	unsigned int w;
	static reg = INI_REG;

	fprintf(stderr,"%02X%c",c,(c!=40)?' ':'\n');

	if ((c>=32) && (c < 40)) reg = c;

	if (c == 40) cnt = 0;

	chars[cnt--] = c;

	if (cnt < 0) {cnt = 2;
				   w = chars[2] + 40*chars[1] + 1600*chars[0];
				   printf( "$%04X%c", w, (c==40)?'\n':',');
				   chars[0]=reg; chars[1]=reg; chars[2]=reg;
				   dcount++;
				   dcount++;
				   }
}

main()
{
		char *l;
		int reg = INI_REG, new_reg;

		while (gets(line) != NULL)
		{
			fprintf (stderr, "%s  ", line);
			printf("\t\t\t;%s\n", line);
			printf("\t\tdw\t"); reg = INI_REG;
			for (l = line; *l; l++, scount++)
			{
				if ((*l == 32) && (reg < 4)) new_reg = reg;
				else new_reg = (*l >> 5) & 7;
				if (new_reg != reg) {reg = new_reg; pack (new_reg + 32);}
				pack (*l & 31);
			}
			pack (40); scount++;
		}
		fprintf (stderr, "\nSource: %d, Destination: %d\n", scount, dcount);
}