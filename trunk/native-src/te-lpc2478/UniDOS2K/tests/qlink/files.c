#include "qlink.h"

struct ObjFile *filelist = NULL;
struct Function *functab[FBUCKETS];

void addobjectlibfile(char *name)
{
  addobjectfile(name);
}

void addobjectfile(char *name)
{
  unsigned char *file;
  FILE *f;
  struct ObjFile *zobjf;
  int len;

  openfile = f = fopen(name, "rb");
  if(!f) {
    settfile(name);
    fatal("QObject file `%s' not found", name);
  }
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  file = getmem(len+16);
  fseek(f, 0, SEEK_SET);
  fread(file, 1, len, f);
  fclose(f);
  openfile = NULL;

  zobjf = getmem(sizeof(struct ObjFile));

  zobjf->filename = addstr(&bstrings, name);
  zobjf->data = file;

  if(*(int *)file != QOBJ_ID) {
    settfile(name);
    fatal("File `%s' is not QObject", name);
  }

  /* link object file */
  zobjf->next = filelist;
  filelist = zobjf;
}

void makefnlist(void)
{
  struct ObjFile *of = filelist;
  int i, tabpos;
  int nfuncs, fnum;
  char *dat, *funchead, *fstrings;
  int *idat, *ifunchead;
  struct Function *zfn, *afn;
  char *fname;

  for(i = 0; i < FBUCKETS; i++) {
    functab[i] = NULL;
  }
  while(of) {
    dat = of->data;
    idat = (int *)dat;
    nfuncs = idat[2];
    if(verbose)
      fprintf(stderr, "%s: QObject file `%s' holds %d %s\n",
                progname,
                getstr(&bstrings, of->filename),
                nfuncs,
                ((nfuncs == 1) ? "symbol" : "symbols") );
    funchead = dat + idat[3];
    ifunchead = (int *)funchead;
    fstrings = dat + idat[9];
    for(fnum = 0; fnum < nfuncs; fnum++) {
      zfn = getmem(sizeof(struct Function));
      fname = fstrings + ifunchead[0];
      zfn->name = addstr(&bstrings, fname);
      zfn->objfile = of;
      zfn->funcnum = fnum;
      zfn->seg = UNUSED;
      zfn->scope = ifunchead[1] & 0xff00;
      zfn->offset = -1;
      zfn->parento = ifunchead[2];
      zfn->fstrings = fstrings;
      zfn->linked = 0;
      tabpos = hash(fname, FBUCKETS);
      zfn->next = functab[tabpos];
      functab[tabpos] = zfn;

      funchead += FHEADSZ;
      ifunchead = (int *)funchead;
    }
    of = of->next;
  }

  /* check for duplicate functions */
  for(i = 0; i < FBUCKETS; i++) {
    zfn = functab[i];
    while(zfn) {
      /* for each func in list */
      /* test name against all later funcs in list */
      afn = zfn->next;
      while(afn) {
        if(!strcmp(getstr(&bstrings, zfn->name), getstr(&bstrings, afn->name))) {
          settfile(getstr(&bstrings, zfn->objfile->filename));
          /* we have two symbols of same name */
          if(zfn->scope == STATIC && afn->scope == STATIC) {
            /* both are static: they must have different static strings */
            if(!strcmp(zfn->fstrings + zfn->parento, afn->fstrings + afn->parento))
              if(multidef)
                warning("Static symbol `%s' multiply defined in `%s'",
                  getstr(&bstrings, zfn->name),
                  getstr(&bstrings, zfn->objfile->filename));
              else
                error("Static symbol `%s' multiply defined in `%s'",
                  getstr(&bstrings, zfn->name),
                  getstr(&bstrings, zfn->objfile->filename));
          } else {
            if(multidef)
              warning("Symbol `%s' multiply defined in `%s' and `%s'",
                getstr(&bstrings, zfn->name),
                getstr(&bstrings, zfn->objfile->filename),
                getstr(&bstrings, afn->objfile->filename));
            else
              error("Symbol `%s' multiply defined in `%s' and `%s'",
                getstr(&bstrings, zfn->name),
                getstr(&bstrings, zfn->objfile->filename),
                getstr(&bstrings, afn->objfile->filename));
          }
        }
        afn = afn->next;
      }
      zfn = zfn->next;
    }
  }
  if(errors)
    fatal("Multiply defined symbols");
}
