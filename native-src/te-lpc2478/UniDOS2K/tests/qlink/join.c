#include "qlink.h"

#define SBUCKETS	128

struct OneString {
  struct OneString *next;
  int off;	/* in ostrings */
};

static struct OneString *onestr[SBUCKETS];


void initjoin(void)
{
  int i;

  for(i = 0; i < SBUCKETS; i++) {
    onestr[i] = NULL;
  }
}

/* add one string only to a bolit */
static int addonestr(char *str)
{
  int h = hash(str, SBUCKETS);
  struct OneString *os, *es;

  os = onestr[h];
  while(os) {
    if(!strcmp(str, bolit.mem + os->off))
      return os->off;
    os = os->next;
  }

  es = getmem(sizeof(struct OneString));
  es->off = addstr(&bolit, str);
  es->next = onestr[h];
  onestr[h] = es;
  return es->off;
}

/* we will use:-
	bolit    to hold all strings (since some strings have been placed in bstrings)
	bdata    to hold file header and func headers
	bdebug   to hold all debug info
	blinkis  to hold all linkinfos
	bocode   to hold all code
   then we link them all into boutput
*/

void dojoin(void)
{
  int i, l, nfuncs = 0;
  struct Function *zfn;
  unsigned char *filedata, *fhead;
  int *fheadi, *filedatai, *fdebugi, *flinkisi;
  char *fname, *fstrings, *fcode, *flinkis, *fdebug;

  if(bdata.used || bdebug.used || blinkis.used || bocode.used || bolit.used)
    fatal("Internal: Block not empty");
  /* make file header */
  addint(&bdata, QOBJ_ID);
  if(creator)
    addint(&bdata, addonestr(creator));
  else
    addint(&bdata, addonestr("qlink QObject file linker Stu Smith"));
  addint(&bdata, 0);		/* num funcs - update later */
  addint(&bdata, HEADERSZ);	/* address function headers */
  if(procos)
    addint(&bdata, addonestr(procos));
  else
    addint(&bdata, addonestr("ARM/RISCOS"));
  addint(&bdata, 0);		/* reserved */
  addint(&bdata, 2);		/* format 2 */
  addint(&bdata, 0);		/* address code    - ul */
  addint(&bdata, 0);		/* length code     - ul */
  addint(&bdata, 0);		/* address strings - ul */
  addint(&bdata, 0);		/* length strings  - ul */
  addint(&bdata, 0);		/* address linkis  - ul */
  addint(&bdata, 0);		/* length linkis   - ul */
  addint(&bdata, 0);		/* address debug   - ul */
  addint(&bdata, 0);		/* length debug    - ul */

  if(verbose)
    fprintf(stderr, "%s: Joining symbols\n", progname);

  /* now go through all functions is functab and join them */
  for(i = 0; i < FBUCKETS; i++) {
    zfn = functab[i];
    while(zfn) {
      nfuncs++;
      fname = getstr(&bstrings, zfn->name);
      if(verbose)
        fprintf(stderr, "%s:  Including symbol `%s'\n", progname, fname);
      filedata = zfn->objfile->data;
      filedatai = (int *)filedata;
      fhead = filedata + filedatai[3] + (zfn->funcnum) * FHEADSZ;
      fheadi = (int *)fhead;
      fstrings = filedata + filedatai[9];
      fcode = filedata + filedatai[7];
      flinkis = filedata + filedatai[11];
      fdebug = filedata + filedatai[13];
      alignarea(&bocode);
      addint(&bdata, addonestr(fname));
      addint(&bdata, fheadi[1]);
      if(verbose)
        fprintf(stderr, " parento: `%s'\n", fstrings + fheadi[2]);
      addint(&bdata, addonestr(fstrings + fheadi[2]));
      addint(&bdata, fheadi[3]);
      switch(fheadi[3]) {	/* function type */
        case 1:
          addint(&bdata, addtoarea(&bocode, fcode + fheadi[4], fheadi[5]));
          addint(&bdata, fheadi[5]);	/* len */
          addint(&bdata, blinkis.used);
          flinkisi = (int *)(flinkis + fheadi[6]);
          for(l = 0; l < fheadi[7]; l++) {
            addint(&blinkis, flinkisi[0 + l*3]);
            if(verbose)
              fprintf(stderr, " linkto: `%s'\n", fstrings + flinkisi[1 + l*3]);
            addint(&blinkis, addonestr(fstrings + flinkisi[1 + l*3]));
            addint(&blinkis, flinkisi[2 + l*3]);
          }
          addint(&bdata, fheadi[7]);
          break;
        case 2:
          if(verbose)
            fprintf(stderr, " offsetin: `%s'\n", fstrings + fheadi[4]);
          addint(&bdata, addonestr(fstrings + fheadi[4]));
          addint(&bdata, fheadi[5]);	/* offset */
          addint(&bdata, 0);
          addint(&bdata, 0);
          break;
        case 3:
          if(verbose)
            fprintf(stderr, " blocksz: %d\n", fheadi[5]);
          addint(&bdata, 0);		/* unused */
          addint(&bdata, fheadi[5]);	/* size blk */
          addint(&bdata, 0);
          addint(&bdata, 0);
          break;
        default:
          settfile(getstr(&bstrings, zfn->objfile->filename));
          fatal("Symbol `%s' has unknown type", fname);
      }
      if(fheadi[8]) {
        fdebugi = (int *)(fdebug + fheadi[8]);
        switch(fdebugi[0]) {
          case 8:
            addint(&bdata, bdebug.used);
            addint(&bdebug, 8);
            addint(&bdebug, addonestr(fstrings + fdebugi[1]));
            addint(&bdebug, fdebugi[2]);
            break;
          default:
            settfile(getstr(&bstrings, zfn->objfile->filename));
            warning("Can't deal with debug information for symbol `%s'", fname);
            addint(&bdata, 0);
        }
      } else
        addint(&bdata, 0);
      zfn = zfn->next;
    }
  }

  if(nfuncs == 0)
    fatal("No symbols to join");

  /* now join all the parts and update file header */
  alignarea(&bolit);
  alignarea(&bocode);
  alignarea(&bdebug);
  placeint(&bdata, nfuncs, 8);
  placeint(&bdata, addtoarea(&bdata, bocode.mem, bocode.used), 28);
  placeint(&bdata, bocode.used, 32);
  dropblock(&bocode);
  placeint(&bdata, addtoarea(&bdata, blinkis.mem, blinkis.used), 44);
  placeint(&bdata, blinkis.used, 48);
  dropblock(&blinkis);
  placeint(&bdata, addtoarea(&bdata, bolit.mem, bolit.used), 36);
  placeint(&bdata, bolit.used, 40);
  dropblock(&bolit);
  placeint(&bdata, addtoarea(&bdata, bdebug.mem, bdebug.used), 52);
  placeint(&bdata, bdebug.used, 56);
  dropblock(&bdebug);

  if(verbose)
    fprintf(stderr, "%s: Writing file `%s'\n", progname, outputfile);
  saveblock(outputfile, bdata.mem, bdata.used);
}
