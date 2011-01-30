/* RISC OS system-specific routines for qas */

//#include <kernel.h>
#include "qas.h"
#include "throw.h"

#define	OS_FSControl	0x29
#define OS_File		0x08
#define XOS_Bit		0x020000

int verbose = 0, throwback = 0, ythrow = 0, supwarn = 0, pedantic = 0;
char *outputfile = NULL;

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

static void displayhelp(void)
{
  int i;
  static char *help[] = {
    "",
    ": Syntax: ",
    "",
    " [option|sourcefile]...\n\n",
    " By convention, qas source files are held in directory s.\n",
    " QObject files are output into directory q.\n",
    " Options for qas are as follows:-\n",
    "  -limit n    Set the error limit to n.\n",
    "  -o file     Name the output file if only one source file is specified.\n",
    "  -throwback  Use throwback.\n",
    "  -v          Verbose\n",
    "  -w          Suppress warnings\n",
    "  -p          Pedantic\n",
    0};

  help[0] = help[2] = progname;

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
  int i, fcount = 0, a;

  /* test if no args */
  if(argc == 1) {
    displayhelp();
    exit(EXIT_SUCCESS);
  }

  for(i = 1; i < argc; i++) {
    if(*argv[i] == '-') {
      if(!strcmp(argv[i]+1, "v")) {
        fprintf(stderr, "\n%s ARM Assembler producing QObject files\n", argv[0]);
        fprintf(stderr, "%s: Version %s (%s) Stu Smith\n", argv[0], VERS, __DATE__);
        verbose = 1;
      } else if(!strcmp(argv[i]+1, "throwback")) {
        throwback = 1;
      } else if(!strcmp(argv[i]+1, "w")) {
        if(pedantic)
          warning("-w used with -p");
        else
          supwarn = 1;
      } else if(!strcmp(argv[i]+1, "p")) {
        pedantic = 1;
        if(supwarn) {
          supwarn = 0;
          warning("-w used with -p");
        }
      } else if(!strcmp(argv[i]+1, "o")) {
        checknextarg(i, argc, argv, "Syntax: -o filename");
        if(outputfile)
          freemem(outputfile);
        outputfile = getmem(strlen(argv[i+1])+1);
        strcpy(outputfile, argv[i+1]);
        i++;
      } else if(!strcmp(argv[i]+1, "limit")) {
        checknextarg(i, argc, argv, "Syntax: -limit n");
        if(sscanf(argv[i+1], "%d", &a) != 1)
          fatal("Illegal error limit");
        if(a < 1)
          fatal("Limit must be at least 1");
        maxerrors = a;
      } else {
        error("Unknown option `%s'", argv[i]);
      }
    } else {
      addsourcefile(argv[i]);
      fcount++;
    }
  }
  if(fcount == 0)
    if(verbose != 0)
      exit(0);
    else
      fatal("Must specify at least one source file");

  /* check that if fcount>1, no output file specified */
  if(fcount > 1)
    if(outputfile)
      fatal("Output file may only be specified with one source file");
}

void saveblock(char *fn, void *blk, int sz)
{

 FILE *f = fopen(fn, "wb");
 fwrite(blk, 1, sz, f);
 fclose(f);
/*
  _kernel_swi_regs r;
  _kernel_oserror *e;

  r.r[0] = 0x0a;
  r.r[1] = (int) riscos_convert(fn);
  r.r[2] = 0xffd;
  r.r[4] = (int) blk;
  r.r[5] = (int) blk+sz;
  e = _kernel_swi(XOS_Bit | OS_File, &r, &r);
  if(e) {
    error("Can't save object file `%s': %s", fn, e->errmess);
  }
 */
}

void makeoutname(char *buf, char *fn)
{
  char lc = *(fn+strlen(fn)-1);
  char llc = *(fn+strlen(fn)-2);
  char *rep;

  strcpy(buf, fn);

  /* test if path.file.s */
  if((lc == 's' || lc == 'S') && (llc == '.')) {
    *(buf+strlen(buf)-1) = 'q';
    return;
  }

  /* test if path.s.file */
  rep = strstr(buf, ".s.");
  if(rep == NULL)
    rep = strstr(buf, "/s.");
  if(rep == NULL)
    rep = strstr(buf, "/s/");
  if(rep == NULL)
    rep = strstr(buf, ".s/");
  if(rep == NULL)
    rep = strstr(buf, ".S.");
  if(rep) {
    *(rep+1) = 'q';
    return;
  }

  /* test if s.file */
  if((*buf == 's' || *buf == 'S') && *(buf+1) == '.') {
    *buf = 'q';
    return;
  }

  /* can't save otherwise */
  if(outputfile == NULL)
    fatal("Must specify an output filename if source is not in `.s' directory");
}


