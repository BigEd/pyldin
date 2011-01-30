/* RISC OS system-specific routines for qlink */

//#include <kernel.h>
#include "qlink.h"
#include "throw.h"

#define OS_File		0x08
#define XOS_Bit		0x020000

int verbose = 0, multidef = 0, map = 0, join = 0, throwback = 0, ythrow = 0;
int codenames = 0, datanames = 0, pagealign = 0, startaddr = 0x8000;
int mapall = 0, fussy = 0;
char *outputfile = NULL;
char *creator = NULL, *procos = NULL;
char *firstfunction = NULL, *symfile;
static int ftype = 0xff8;
static int fcount = 0, vflag = 0;

char *riscos_convert(char *fn)
{
  static char filename[256];
  char f[256], name[256];
  int len = strlen(fn);
  int sufp = 0, filep = 0, pathp = 0, fp = 0, p;
  int lastper, filenameref;
  char *t, *t2;

  filename[0] = 0;
  assert(strlen(fn) < 255);
  if(strlen(fn) > 3 && !strncmp(fn, "/tmp", 4)) {
    strcpy(f, "<Wimp$ScrapDir>");
    fn += 4;	/* strlen("/tmp") */
    fp += 15;	/* strlen("<Wimp$ScrapDir>") */
  }

  for(p = 0; fn[p]; p++) {
    if(fn[p] == '/')
      f[fp] = '.';
    else
      f[fp] = fn[p];
    fp++;
  }
  f[fp] = 0;

  t = strrchr(f, '.');

  /* no `.', so just return */
  if(t == NULL) {
    strcpy(filename, f);
    return filename;
  }
  lastper = t-f;

  /* lastper = ref last . */
  f[lastper] = 0;
  t  = strrchr(f, '.');
  t2 = strrchr(f, ':');
  if(t2 > t)
    t = t2;
  if(t == NULL)
    filenameref = 0;
  else
    filenameref = t-f+1;

  /*f[lastper] = '.';*/

  lastper++;

  /* lastper is refernce in f to suffix */
  /* filenameref is ref in f to filename */

  /* not a three char suffix */
  if(strlen(f+lastper) > 3) {
    f[lastper-1] = '.';
    strcpy(filename, f);
    return filename;
  }

  strcpy(name, f+filenameref);
  strncpy(filename, f, filenameref);
  filename[filenameref] = 0;
  strcat(filename, f+lastper);
  strcat(filename, ".");
  strcat(filename, name);

  return filename;
}

static void via(char *fname)
{
  FILE *f = fopen(fname, "r");
  char ff[255];

  if(!f)
    fatal("Can't open `%s' to read QObject files", fname);

  while(fscanf(f, "%s", &ff) == 1) {
    addobjectfile(ff);
    fcount++;
  }

  fclose(f);
}

static void displayhelp(void)
{
  int i;
  static char *help[] = {
    "",
    ": Syntax: ",
    "",
    " [option|sourcefile]...\n\n",
    " By convention, QObject files are held in directory q.\n",
    " Options for qlink are as follows:-\n",
    "  -addr address    Set the start address of the output file (default 8000).\n",
    "  -codenames       Insert symbol names before CODE symbols.\n",
    "  -creator string  Set the creator string. (Join only).\n",
    "  -datanames       Insert symbol names before DATA/LIT symbols.\n",
    "  -first function  Set the first function (default __aif).\n",
    "  -fussy           Warn about out-of-range references.\n",
    "  -join            Join the QObject files rather than linking them.\n",
    "  -lib file        Treat file as a library file.\n",
    "  -map             Print a symbol map to stderr.\n",
    "  -mapall          Map all symbols, even pseudo-locals.\n",
    "  -multidef        Allow multiple definitions of symbols.\n",
    "  -o file          Name the output file (default !RunImage).\n",
    "  -procos          Set the processor/OS string. (Join only).\n",
    "  -symbols file    Write a list of symbols to file.\n",
    "  -t               Debug information printed on stderr.\n",
    "  -throwback       Use throwback.\n",
    "  -type type       Set the type of the output file (default FF8).\n",
    "  -v               Verbose.\n",
    0};

  help[2] = help[0] = progname;

  for (i = 0; help[i]; i++) {
    fprintf(stderr, "%s", help[i]);
  }
}

static void checknextarg(int i, int argc, char **argv, char *es)
{
  if(i+1 >= argc)
    fatal(es);
  if(*argv[i+1] == '-')
    fatal(es);
}

void setup(int argc, char **argv)
{
  int i, a, typed = 0, named = 0, libbed = 0, firsted = 0, addred = 0;
  char *s;

  firstfunction = getmem(7);
  strcpy(firstfunction, "__aif");
  outputfile = getmem(10);
  strcpy(outputfile, "!RunImage");
  /* test if no args */
  if(argc == 1) {
    displayhelp();
    exit(EXIT_SUCCESS);
  }

  for(i = 1; i < argc; i++) {
    if(*argv[i] == '-') {
      if(!strcmp(argv[i]+1, "t")) {
        /* debug */
        verbose = 1;
      } else if(!strcmp(argv[i]+1, "v")) {
        /* verbose */
        fprintf(stderr, "\n%s QObject file linker\n", argv[0]);
        fprintf(stderr, "%s: Version %s (%s) Stu Smith\n", argv[0], VERS, __DATE__);
        vflag = 1;
      } else if(!strcmp(argv[i]+1, "o")) {
        /* outputfile */
        named = 1;
        checknextarg(i, argc, argv, "Syntax: -o filename");
        if(outputfile)
          freemem(outputfile);
        outputfile = getmem(strlen(argv[i+1])+1);
        strcpy(outputfile, argv[i+1]);
        i++;
      } else if(!strcmp(argv[i]+1, "lib")) {
        /* lib */
        libbed = 1;
        checknextarg(i, argc, argv, "Syntax: -lib file");
        addobjectlibfile(argv[i+1]);
        fcount++;
        i++;
      } else if(!strcmp(argv[i]+1, "via")) {
        /* via */
        checknextarg(i, argc, argv, "Syntax: -via file");
        via(argv[i+1]);
        i++;
      } else if(!strcmp(argv[i]+1, "multidef")) {
        /* multidef */
        multidef = 1;
      } else if(!strcmp(argv[i]+1, "mapall")) {
        /* mapall */
        mapall = 1;
      } else if(!strcmp(argv[i]+1, "fussy")) {
        /* fussy */
        fussy = 1;
      } else if(!strcmp(argv[i]+1, "first")) {
        /* first */
        checknextarg(i, argc, argv, "Syntax: -first function");
        firsted = 1;
        if(firstfunction)
          freemem(firstfunction);
        firstfunction = getmem(strlen(argv[i+1])+1);
        strcpy(firstfunction, argv[i+1]);
        i++;
      } else if(!strcmp(argv[i]+1, "type")) {
        /* type */
        typed = 1;
        checknextarg(i, argc, argv, "Syntax: -type type");
        if(sscanf(argv[i+1], "%x", &a) != 1)
          fatal("Illegal type");
        if(a > 0xfff)
          fatal("Type must be in the range 0 - 0xfff");
        ftype = a;
        i++;
      } else if(!strcmp(argv[i]+1, "addr")) {
        /* addr */
        addred = 1;
        checknextarg(i, argc, argv, "Syntax: -addr address");
        if(sscanf(argv[i+1], "%x", &a) != 1)
          fatal("Illegal address");
        startaddr = a;
        i++;
      } else if(!strcmp(argv[i]+1, "codenames")) {
        /* codenames */
        codenames = 1;
      } else if(!strcmp(argv[i]+1, "join")) {
        /* join */
        join = 1;
      } else if(!strcmp(argv[i]+1, "datanames")) {
        /* datanames */
        datanames = 1;
      } else if(!strcmp(argv[i]+1, "pagealign")) {
        /* pagealign */
        pagealign = 1;
      } else if(!strcmp(argv[i]+1, "throwback")) {
        throwback = 1;
        errors = 0;
      } else if(!strcmp(argv[i]+1, "map")) {
        map = 1;
      } else if(!strcmp(argv[i]+1, "symbols")) {
        /* symbols */
        checknextarg(i, argc, argv, "Syntax: -symbols file");
        if(symfile)
          freemem(symfile);
        symfile = getmem(strlen(argv[i+1])+1);
        strcpy(symfile, argv[i+1]);
        i++;
      } else if(!strcmp(argv[i]+1, "creator")) {
        /* creator */
        checknextarg(i, argc, argv, "Syntax: -creator string");
        if(creator)
          freemem(creator);
        s = argv[i+1];
        if(*s == '"') {
          s++;
          if(*(s+strlen(s)-1) != '"')
            fatal("Unmatched quote");
          *(s+strlen(s)-1) = 0;
        }
        creator = getmem(strlen(s)+1);
        strcpy(creator, s);
        i++;
      } else if(!strcmp(argv[i]+1, "procos")) {
        /* procos */
        checknextarg(i, argc, argv, "Syntax: -procos string");
        if(procos)
          freemem(procos);
        s = argv[i+1];
        if(*s == '"') {
          s++;
          if(*(s+strlen(s)-1) != '"')
            fatal("Unmatched quote");
          *(s+strlen(s)-1) = 0;
        }
        procos = getmem(strlen(s)+1);
        strcpy(procos, s);
        i++;
      } else {
        fatal("Unknown option `%s'", argv[i]);
      }
    } else {
      addobjectfile(argv[i]);
      fcount++;
    }
  }
  if(fcount == 0)
    if(vflag != 0)
      exit(0);
    else
      fatal("Must specify at least one QObject file");

  if(map && symfile)
    fatal("Only one of -symbols and -map may be specified");

  if(join) {
    if(!named) {
      freemem(outputfile);
      outputfile = getmem(7);
      strcpy(outputfile, "q.join");
    }
    if(!typed)
      ftype = 0xffd;
    if(libbed || multidef || codenames || datanames || symfile || map
    || firsted || addred)
      fatal("Illegal option used with -join");
  } else {
    if(procos || creator)
      fatal("Can't specify -creator or -procos without -join");
  }
}

void saveblock(char *fn, void *blk, int sz)
{
 FILE *f = fopen(fn, "wb");
 fwrite(blk, 1, sz, f);
 fclose(f);
}


void settfile(char *f)
{
  if(throwback && ythrow) {
//    throw_file(f);
  }
}
