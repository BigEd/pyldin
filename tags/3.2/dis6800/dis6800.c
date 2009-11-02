#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
   unsigned char opcode;
   unsigned char numoperands;
   char name[6];
   unsigned char mode;
   unsigned char numcycles;
} opcodeinfo;
   
/* 6800 ADDRESSING MODES */
#define INH 0        
#define DIR 1
#define IND 2
#define REL 3
#define EXT 4
#define IMM 5

#define NUMOPS 197

static opcodeinfo opcodes[NUMOPS]={
   {1,0,"nop",INH,2},
   {6,0,"tap",INH,2},
   {7,0,"tpa",INH,2},
   {8,0,"inx",INH,3},
   {9,0,"dex",INH,3},
   {10,0,"clv",INH,2},
   {11,0,"sev",INH,2},
   {12,0,"clc",INH,2},
   {13,0,"sec",INH,2},
   {14,0,"cli",INH,2},
   {15,0,"sei",INH,2},
   
   {16,0,"sba",INH,2},
   {17,0,"cba",INH,2},
   {22,0,"tab",INH,2},
   {23,0,"tba",INH,2},
   {25,0,"daa",INH,2},
   {27,0,"aba",INH,2},

   {32,1,"bra",REL,3},
   {34,1,"bhi",REL,3},
   {35,1,"bls",REL,3},
   {36,1,"bcc",REL,3},
   {37,1,"bcs",REL,3},
   {38,1,"bne",REL,3},
   {39,1,"beq",REL,3},
   {40,1,"bvc",REL,3},
   {41,1,"bvs",REL,3},
   {42,1,"bpl",REL,3},
   {43,1,"bmi",REL,3},
   {44,1,"bge",REL,3},
   {45,1,"blt",REL,3},
   {46,1,"bgt",REL,3},
   {47,1,"ble",REL,3},

   {48,0,"tsx",INH,3},
   {49,0,"ins",INH,3},
   {50,0,"pula",INH,4},
   {51,0,"pulb",INH,4},
   {52,0,"des",INH,3},
   {53,0,"txs",INH,3},
   {54,0,"psha",INH,3},
   {55,0,"pshb",INH,3},
   {57,0,"rts",INH,5},
   {59,0,"rti",INH,12},
   {62,0,"wai",INH,14},
   {63,1,"int",DIR,2},
   
   {64,0,"nega",INH,2},
   {67,0,"coma",INH,2},
   {68,0,"lsra",INH,2},
   {70,0,"rora",INH,2},
   {71,0,"asra",INH,2},
   {72,0,"asla",INH,2},
   {73,0,"rola",INH,2},
   {74,0,"deca",INH,2},
   {76,0,"inca",INH,2},
   {77,0,"tsta",INH,2},
   {79,0,"clra",INH,2},

   {80,0,"negb",INH,2},
   {83,0,"comb",INH,2},
   {84,0,"lsrb",INH,2},
   {86,0,"rorb",INH,2},
   {87,0,"asrb",INH,2},
   {88,0,"aslb",INH,2},
   {89,0,"rolb",INH,2},
   {90,0,"decb",INH,2},
   {92,0,"incb",INH,2},
   {93,0,"tstb",INH,2},
   {95,0,"clrb",INH,2},
   
   {96,1,"neg",IND,6},
   {99,1,"com",IND,6},
   {100,1,"lsr",IND,6},
   {102,1,"ror",IND,6},
   {103,1,"asr",IND,6},
   {104,1,"asl",IND,6},
   {105,1,"rol",IND,6},
   {106,1,"dec",IND,6},
   {108,1,"inc",IND,6},
   {109,1,"tst",IND,6},
   {110,1,"jmp",IND,3},
   {111,1,"clr",IND,6},

   {112,2,"neg",EXT,6},
   {115,2,"com",EXT,6},
   {116,2,"lsr",EXT,6},
   {118,2,"ror",EXT,6},
   {119,2,"asr",EXT,6},
   {120,2,"asl",EXT,6},
   {121,2,"rol",EXT,6},
   {122,2,"dec",EXT,6},
   {124,2,"inc",EXT,6},
   {125,2,"tst",EXT,6},
   {126,2,"jmp",EXT,3},
   {127,2,"clr",EXT,6},
   
   {128,1,"suba",IMM,2},
   {129,1,"cmpa",IMM,2},
   {130,1,"sbca",IMM,2},
   {132,1,"anda",IMM,2},
   {133,1,"bita",IMM,2},
   {134,1,"ldaa",IMM,2},
   {136,1,"eora",IMM,2},
   {137,1,"adca",IMM,2},
   {138,1,"oraa",IMM,2},
   {139,1,"adda",IMM,2},
   {140,2,"cpx",IMM,4},
   {141,1,"bsr",REL,6},
   {142,2,"lds",IMM,3},
   
   {144,1,"suba",DIR,3},
   {145,1,"cmpa",DIR,3},
   {146,1,"sbca",DIR,3},
   {148,1,"anda",DIR,3},
   {149,1,"bita",DIR,3},
   {150,1,"ldaa",DIR,3},
   {151,1,"staa",DIR,3},
   {152,1,"eora",DIR,3},
   {153,1,"adca",DIR,3},
   {154,1,"oraa",DIR,3},
   {155,1,"adda",DIR,3},
   {156,1,"cpx",DIR,5},
   {158,1,"lds",DIR,4},
   {159,1,"sts",DIR,4},
   
   {160,1,"suba",IND,4},
   {161,1,"cmpa",IND,4},
   {162,1,"sbca",IND,4},
   {164,1,"anda",IND,4},
   {165,1,"bita",IND,4},
   {166,1,"ldaa",IND,4},
   {167,1,"staa",IND,4},
   {168,1,"eora",IND,4},
   {169,1,"adca",IND,4},
   {170,1,"oraa",IND,4},
   {171,1,"adda",IND,4},
   {172,1,"cpx",IND,6},
   {173,1,"jsr",IND,6},
   {174,1,"lds",IND,5},
   {175,1,"sts",IND,5},
   
   {176,2,"suba",EXT,4},
   {177,2,"cmpa",EXT,4},
   {178,2,"sbca",EXT,4},
   {180,2,"anda",EXT,4},
   {181,2,"bita",EXT,4},
   {182,2,"ldaa",EXT,4},
   {183,2,"staa",EXT,4},
   {184,2,"eora",EXT,4},
   {185,2,"adca",EXT,4},
   {186,2,"oraa",EXT,4},
   {187,2,"adda",EXT,4},
   {188,2,"cpx",EXT,6},
   {189,2,"jsr",EXT,6},
   {190,2,"lds",EXT,5},
   {191,2,"sts",EXT,5},
   
   {192,1,"subb",IMM,2},
   {193,1,"cmpb",IMM,2},
   {194,1,"sbcb",IMM,2},
   {196,1,"andb",IMM,2},
   {197,1,"bitb",IMM,2},
   {198,1,"ldab",IMM,2},
   {200,1,"eorb",IMM,2},
   {201,1,"adcb",IMM,2},
   {202,1,"orab",IMM,2},
   {203,1,"addb",IMM,2},
   {206,2,"ldx",IMM,3},
   
   {208,1,"subb",DIR,3},
   {209,1,"cmpb",DIR,3},
   {210,1,"sbcb",DIR,3},
   {212,1,"andb",DIR,3},
   {213,1,"bitb",DIR,3},
   {214,1,"ldab",DIR,3},
   {215,1,"stab",DIR,3},
   {216,1,"eorb",DIR,3},
   {217,1,"adcb",DIR,3},
   {218,1,"orab",DIR,3},
   {219,1,"addb",DIR,3},
   {222,1,"ldx",DIR,4},
   {223,1,"stx",DIR,4},
   
   {224,1,"subb",IND,4},
   {225,1,"cmpb",IND,4},
   {226,1,"sbcb",IND,4},
   {228,1,"andb",IND,4},
   {229,1,"bitb",IND,4},
   {230,1,"ldab",IND,4},
   {231,1,"stab",IND,4},
   {232,1,"eorb",IND,4},
   {233,1,"adcb",IND,4},
   {234,1,"orab",IND,4},
   {235,1,"addb",IND,4},
   {238,1,"ldx",IND,5},
   {239,1,"stx",IND,5},
   
   {240,2,"subb",EXT,4},
   {241,2,"cmpb",EXT,4},
   {242,2,"sbcb",EXT,4},
   {244,2,"andb",EXT,4},
   {245,2,"bitb",EXT,4},
   {246,2,"ldab",EXT,4},
   {247,2,"stab",EXT,4},
   {248,2,"eorb",EXT,4},
   {249,2,"adcb",EXT,4},
   {250,2,"orab",EXT,4},
   {251,2,"addb",EXT,4},
   {254,2,"ldx",EXT,5},
   {255,2,"stx",EXT,4},
};

static FILE *srcFile;
static FILE *dstFile;

opcodeinfo *getOpCode(unsigned char opcode)
{
    int i;
    
    for (i = 0; i < NUMOPS; i++) {
	if (opcodes[i].opcode == opcode)
	    return &opcodes[i];
    }
    
    return NULL;
}

unsigned char getByte(void)
{
    return fgetc(srcFile);
}

unsigned short getWord(void)
{
    unsigned char h = fgetc(srcFile);
    unsigned char l = fgetc(srcFile);
    
    return (h << 8) | l;
}

unsigned short disassembly(unsigned short addr)
{
    unsigned char byte = getByte();
    opcodeinfo *op = getOpCode(byte);
    
    if (!op) {
	fprintf(dstFile, "\tdb\t$%X\t; Unknown opcode\n", byte);
	return addr + 1;
    }
    
    fprintf(dstFile, "\t%s", op->name);
    if (op->numoperands) {
	char d_s[128];
	unsigned short d = (op->numoperands == 1)?getByte():getWord();
	if (op->numoperands == 1)
	    sprintf(d_s, "%02X", d);
	else
	    sprintf(d_s, "%04X", d);
	
	switch (op->mode) {
	    case INH:
		break;
	    case DIR:
		fprintf(dstFile, "\t$%s", d_s);
		break;
	    case IND:
		fprintf(dstFile, "\t$%02X,x", d);
		break;
	    case REL:
		fprintf(dstFile, "\t$%04X", ((char)d) + addr + op->numoperands + 1);
		break;
	    case EXT:
		fprintf(dstFile, "\t$%s", d_s);
		break;
	    case IMM:
		fprintf(dstFile, "\t#$%s", d_s);
		break;
	}
    }
    addr += op->numoperands + 1;
    fprintf(dstFile, "\n");

    if (!strcmp(op->name, "rts"))
	fprintf(dstFile, "\n");    

    if (!strcmp(op->name, "jmp"))
	fprintf(dstFile, "\n");    
    
    return addr;
}

void usage(char *app)
{
    fprintf(stderr, "Usage: %s [-h][-s startaddr][-o outputfile] inputfile\n", app);
    exit(0);
}

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, optopt, opterr;
    int c;
    unsigned short addr, startaddr = 0;

    dstFile = stdout;

    while ((c = getopt(argc, argv, ":ho:s:")) != -1)
    {
        switch (c)
        {
	case 'h':
	    usage(argv[0]);
	    break;
        case 'o':
	    dstFile = fopen(optarg, "wb");
	    if (!dstFile) {
		fprintf(stderr, "Can't open output file!\n");
		return 1;
	    }
            break;
	case 's': {
		int tmp;
	        sscanf(optarg, "%x", &tmp);
		startaddr = tmp;
	    }
	    break;
        default:
	    usage(argv[0]);
            break;
        }
    }

    if (optind == argc)
	usage(argv[0]);

    for ( ; optind < argc; optind++) {
	addr = startaddr;
	fprintf(dstFile, ";\n; %s\n;\n", argv[optind]);
	srcFile = fopen(argv[optind], "rb");
	if (!srcFile) {
	    fprintf(stderr, "Can't open input file!\n");
	    return 1;
	}
	
	fprintf(dstFile, "\n\t\torg\t$%X\n\n", addr);
	
	while(!feof(srcFile)) {
	    fprintf(dstFile, "%04X\t", addr);
	    addr = disassembly(addr);
	}
	fclose(srcFile);
	fprintf(dstFile, "\n\n\n");
    }

    fclose(dstFile);

    return 0;
}
