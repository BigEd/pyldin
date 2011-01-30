#include <limits.h>
#include "qlink.h"

#define SYM_FORM	"%-31s %-7s  %-6s  %06X   %s\n"
#define SYM_FORM2	"%-31s %-7s  %-6s  %-7s  %s\n"

struct sinfo {
  struct sinfo *next;
  struct sinfo *prev;
  char *name;
  int segment;
  int scope;
  int off;
  char *fname;
};

static struct sinfo *sinfos = NULL;

static char *segnames[] = {"<SEG=0>", "CODE", "BSS", "DATA", "LIT" };
static char *scopenames[] = {"<SCOPE=0>", "GLOBAL", "STATIC" };

static char *fffn;

static int bsssize = 0, codesize = 0, datasize = 0, litsize = 0;
static int codeoff = 0, dataoff = 0, litoff = 0, bssoff = 0;
static FILE *symf = NULL;
static int numnotprinted, numprinted;
static int undefined;

static void printlm(void)
{
  int a;
  struct sinfo *si, *sl;

  /* print min. member of list until no entries in list */

  while(sinfos) {
    a = sinfos->off;
    sl = sinfos;
    si = sinfos->next;
    while(si) {
      if(si->off < a) {
        sl = si;
        a = sl->off;
      }
      si = si->next;
    }
    fprintf(symf, SYM_FORM,
                   sl->name,
                   segnames[sl->segment],
                   scopenames[(sl->scope) >> 8],
                   sl->off,
                   sl->fname);
    numprinted++;
    if(sl->prev)
      sl->prev->next = sl->next;
    else
      sinfos = sl->next;
    if(sl->next)
      sl->next->prev = sl->prev;
    freemem(sl);
  }
}

static int builtin(char *name)
{
  int v;

  if(!strcmp(name, "StartAddr"))        v = startaddr;
  else if(!strcmp(name, "EndAddr"))	v = bssoff + bsssize + startaddr;
  else if(!strcmp(name, "CODE_Offset")) v = codeoff;
  else if(!strcmp(name, "DATA_Offset")) v = dataoff;
  else if(!strcmp(name, "BSS_Offset"))  v = bssoff;
  else if(!strcmp(name, "LIT_Offset"))  v = litoff;
  else if(!strcmp(name, "CODE_Size"))   v = codesize;
  else if(!strcmp(name, "DATA_Size"))   v = datasize;
  else if(!strcmp(name, "BSS_Size"))    v = bsssize;
  else if(!strcmp(name, "LIT_Size"))    v = litsize;
  else if(!strcmp(name, "ROBase"))      v = startaddr;
  else if(!strcmp(name, "ROLimit"))     v = dataoff + startaddr;
  else if(!strcmp(name, "RWBase"))      v = dataoff + startaddr;
  else if(!strcmp(name, "RWLimit"))     v = bssoff + bsssize + startaddr;
  else if(!strcmp(name, "RO_Size"))     v = codesize + litsize;
  else
    fatal("Unknown built-in symbol `Q$$%s'", name);
  return v - startaddr;
}

static struct Function *findfunc(char *fname, char *obf)
{
  struct Function *afn = functab[hash(fname, FBUCKETS)];

  while(afn) {
    if(!strcmp(fname, getstr(&bstrings, afn->name))) {
      /* obf must be NULL or same as functions if func is static */
      if( (obf == NULL)
      ||  (afn->scope == GLOBAL)
      ||  (!strcmp(obf, afn->fstrings + afn->parento)) )
        return afn;
    }
    afn = afn->next;
  }
  return NULL;
}

static enum Segment getfuncseg(char *name, char *obf)
{
  struct Function *fn = findfunc(name, obf);

  if(fn == NULL)
    fatal("Internal error: Can't find symbol `%s'", name);
  if(fn->seg < 1 || fn->seg > 4) {
    settfile(getstr(&bstrings, fn->objfile->filename));
    fatal("Internal error: Symbol `%s' in bad segment (%d)", name, fn->seg);
  }
  return fn->seg;
}

static int *getfhead(struct Function *afn)
{
  unsigned char *d;
  int *di;

  assert(afn);
  d = afn->objfile->data;
  assert(d);
  di = (int *)d;
  d += di[3] + ((afn->funcnum) * FHEADSZ);
  return (int *)d;
}

static int linkfunc(char *fname, char *parent)
{
  int *fhead, *fht, i, *id, off, at, inc, zoff, o, ltype, omin, omax;
  struct Function *afn, *zfn = findfunc(fname, parent);
  unsigned char *fstrings, *flinkis, *d;
  char *child;
  unsigned int instr, intype, instr2, instr3, r;
  struct sinfo *sinfop;

  if(fname[0] == 'Q' && fname[1] == '$' && fname[2] == '$')
    return builtin(fname+3);

  if(zfn == NULL)
    fatal("Internal error: Can't find symbol `%s'", fname);

  settfile(getstr(&bstrings, zfn->objfile->filename));

  if(zfn->offset == -1)
    fatal("Internal error: Symbol `%s' not joined", fname);
  fhead = getfhead(zfn);

  switch(zfn->seg) {
    case CODE: zoff = codeoff; break;
    case BSS:  zoff = bssoff;  break;
    case DATA: zoff = dataoff; break;
    case LIT:  zoff = litoff;  break;
    default:
      fatal("Symbol `%s' has unknown segment %d", fname, zfn->seg);
  }

  if(zfn->linked)
    return zfn->offset + zoff;

  /* if necessary, add to symbol info list for link map */

  if(symf) {
    if(fname[0] != 'L' || fname[1] != '.' || mapall) {
      sinfop = getmem(sizeof(struct sinfo));
      sinfop->name = fname;
      sinfop->segment = zfn->seg;
      sinfop->scope = zfn->scope;
      sinfop->off = zfn->offset + zoff + startaddr;
      sinfop->fname = zfn->fstrings + zfn->parento;
      sinfop->prev = NULL;
      if(sinfos)
        sinfos->prev = sinfop;
      sinfop->next = sinfos;
      sinfos = sinfop;
    } else
      numnotprinted++;
  }

  zfn->linked = 1;

  d = zfn->objfile->data;
  assert(d);
  id = (int *) d;
  fstrings = d + id[9];
  flinkis = d + id[11];

  switch(fhead[3]) {
    case 1:
      for(i = 0; i < fhead[7]; i++) {
        at  = *(int *)(flinkis + fhead[6] + i*LINKISZ + 0);
        ltype = at & 0xff000000;
        at &= 0x00ffffff;
        at += zfn->offset + zoff;		/* at = offset in boutput */
        inc = *(int *)(flinkis + fhead[6] + i*LINKISZ + 8); /* inc = off in child */
        child = fstrings + *(int *)(flinkis + fhead[6] + i*LINKISZ + 4);
        instr = getint(&boutput, at);
        intype = instr & 0x0fff0000;
        omin = INT_MIN;
        omax = INT_MAX;

        if((ltype == 0x01000000)
        && (findfunc(child, zfn->fstrings + zfn->parento) == NULL)) {
          /* weak symbol and not found */
          if(intype == 0x0ddd0000 || intype == 0x0cff0000)
            placeint(&boutput, 0, at);
          else
            fatal("Weak symbols may only be linked using DWD or DWL");
        } else {
          if((afn = findfunc(child, zfn->fstrings + zfn->parento)) != NULL) {
            fht = getfhead(afn);
            if(fht[3] == 1 || fht[3] == 3) {
              omin = 0;
              omax = fht[5];
            }
          }
          if(fussy && (intype != 0x0cff0000) && (inc < omin || inc >= omax))
            warning("Reference to `%s' in `%s' (in `%s') out of range",
                     child, fname, parent ? parent : fffn);

          /* weak & found or strong */
          if(intype != 0x0cff0000)
            off = linkfunc(child, zfn->fstrings + zfn->parento);
          				/* off = offset in boutput of child */

          switch(intype) {
            case 0x0ddd0000:	/* DFN */
              instr = off + inc + startaddr;
              break;
            case 0x0cff0000:	/* DFL */
              afn = findfunc(child, zfn->fstrings + zfn->parento);
              if(afn) {
                fht = getfhead(afn);
                if(fht[3] == 1 || fht[3] == 3) {
                  instr = fht[5] + inc;
                } else {
                  fatal("Can't declare length of symbol `%s' referenced in `%s' (in `%s')",
                    child, fname, parent ? parent : fffn);
                }
              } else
                fatal("Can't find symbol `%s' referenced by `%s' (in `%s')",
                        child, fname, parent ? parent : fffn);
              break;
            case 0x0bff0000:	/* bl */
            case 0x0aff0000:	/* b */
              if((off+inc) & 3)
                fatal("Address not word aligned in branch to symbol `%s' in `%s' (in `%s')",
                  child, fname, parent ? parent : fffn);
              instr &= 0xff000000;
              o = off+inc - at;
              if(o > 0x2000000 || o < -0x2000000)
                fatal("Address out of range in branch to symbol `%s' in `%s' (in `%s')",
                        child, fname, parent ? parent : fffn);
              instr |= ((o>>2)-2) & 0x00ffffff;
              break;
            case 0x0eee0000:	/* adr */
              o = (off+inc - at)-8;
              instr = instr & 0xf0000000;
              if(o < 0) {
                o = -o;
                instr |= (2<<21) | (1<<25);
              } else
                instr |= (4<<21) | (1<<25);
              instr2 = instr;
              instr |= 15<<16;
              r = getint(&boutput, at+4);
              if(r < 0 || r > 15)
                fatal("ADR register out of range in adr to symbol `%s' in `%s' (in `%s')",
                        child, fname, parent ? parent : fffn);
              instr |= r<<12;
              instr2 |= (r<<12) | (r<<16);
              instr3 = instr2;
              instr |= o & 0xff;
              if(o & 0xff00)
                instr2 |= ((o & 0xff00)>>8) | 0xc00;
              if(o & 0xff0000)
                instr3 |= ((o & 0xff0000)>>16) | 0x800;
              placeint(&boutput, instr2, at+4);
              placeint(&boutput, instr3, at+8);
              break;
            default:		/* assume load/store */
              instr &= 0xfffff000;
              o = (off+inc - at) - 8;
              if(o < 0) {
                o = -o;
                instr &= ~(1<<23);
              } else
                instr |= 1<<23;
              if(o > 0xfff)
                fatal("Address out of range in dtrans to symbol `%s' in `%s' (in `%s')",
                        child, fname, parent ? parent : fffn);
              instr |= o & 0xfff;
          }
          placeint(&boutput, instr, at);
        }
      }
      break;
    case 2:
      child = fstrings + fhead[4];
      linkfunc(child, zfn->fstrings + zfn->parento);
      break;
    case 3:
      break;
    default:
      fatal("Internal: Symbol `%s' has unknown type #%d", fname, fhead[3]);
  }
  return zfn->offset + zoff;
}

static void getfunc(char *fname, char *parent, char *pfname)
{
  struct Function *cfn, *zfn = findfunc(fname, parent);
  struct Block *blk;
  int *fhead;
  unsigned char *d, *fstrings, *flinkis, *fcode;
  int *id;
  int i, off, ltype, posn;
  char *pname, *child;

  if(fname[0] == 'Q' && fname[1] == '$' && fname[2] == '$')
    return;

  settfile(parent ? parent : "Command line");

  if(zfn == NULL) {
    undefined++;
    error("Can't find symbol `%s' referenced by `%s' (in `%s')", fname, pfname,
             parent ? parent : fffn);
    return;
  }

  if(zfn->seg != 0)
    return;

  /* add function to appropriate block */
  fhead = getfhead(zfn);
  zfn->seg = fhead[1] & 0xff;

  switch(zfn->seg) {
    case CODE: blk = &bocode; break;
    case BSS:  blk = NULL;    break;
    case DATA: blk = &bodata; break;
    case LIT:  blk = &bolit;  break;
    default: fatal("Symbol `%s' in unknown segment #%d", fname, zfn->seg);
  }

  switch(fhead[3]) {
    case 1:	/* standard */
      if(blk == NULL)
        error("Can't put initialised symbol `%s' in BSS segment", fname);
      else {
        /* copy function to block */
        d = zfn->objfile->data;
        assert(d);
        id = (int *) d;
        fstrings = d + id[9];
        flinkis = d + id[11];
        fcode = d + id[7];
        alignarea(blk);
        if(parent)	/* ie not first function */
          if((codenames && (zfn->seg == CODE))
          || (datanames && (zfn->seg != CODE))) {
            addstr(blk, fname);
            alignarea(blk);
            addint(blk, 0xff000000 | ((strlen(fname)+4) & 0xfffffffc));
          }
        zfn->offset = getdata(blk, fcode, fhead[4], fhead[5]);
        /* load sub-symbols */
        if(verbose)
          fprintf(stderr, "%s: Symbol `%s' references %d %s\n", progname,fname,fhead[7],
                            ((fhead[7] == 1) ? "symbol" : "symbols"));
        for(i = 0; i < fhead[7]; i++) {
          child = (char *)fstrings + *(int *)(flinkis + fhead[6] + i*LINKISZ + 4);
          if(verbose)
            fprintf(stderr, "%s: Symbol `%s' references symbol `%s'\n",
                      progname,
                      fname,
                      child);
          ltype = *(int *)(flinkis + fhead[6] + i*LINKISZ + 0);
          posn = ltype & 0x00ffffff;
          ltype &= 0xff000000;
          if((getint(blk, zfn->offset + posn) & 0x0fff0000) != 0x0cff0000) {
            switch(ltype) {
              case 0x00000000:	/* strong */
                getfunc(child, zfn->fstrings + zfn->parento, fname);
                break;
              case 0x01000000:	/* weak - only if found */
                if(findfunc(child, zfn->fstrings + zfn->parento)) {
                  getfunc(child, zfn->fstrings + zfn->parento, fname);
                } else
                  if(verbose)
                    fprintf(stderr, "%s: Weak symbol `%s' not found\n", progname,child);
                break;
              default:
                error("Unknown link type in symbol `%s'", fname);
            }
          } else {
            if(ltype != 0x01000000 && !findfunc(child, zfn->fstrings + zfn->parento)) {
              undefined++;
              error("Can't find symbol `%s' referenced by `%s' (in `%s')",
                       child, fname, getstr(&bstrings, findfunc(fname, parent)->objfile->filename));
            }
          }
        }
      }
      break;
    case 2:	/* name + offset */
      if(blk == NULL)
        error("Can't put offset symbol `%s' in BSS segment", fname);
      else {
        d = zfn->objfile->data;
        id = (int *) d;
        fstrings = d + id[9];
        pname = fstrings + fhead[4];
        off = fhead[5];
        alignarea(blk);

        if(verbose)
          fprintf(stderr, "%s: Symbol `%s' is offset within symbol `%s'\n",
                    progname,
                    fname,
                    pname);
        if(zfn->seg != (getfhead(findfunc(pname, zfn->fstrings+zfn->parento))[1]&0xff))
          error("Child offset symbol `%s' not in same segment as parent `%s' (%x, %x)",
            fname, pname,
            zfn->seg, (findfunc(pname, zfn->fstrings + zfn->parento)->seg) & 0xff);
        cfn = findfunc(pname, zfn->fstrings + zfn->parento);
        assert(cfn);
        zfn->offset = 0;
        getfunc(pname, zfn->fstrings + zfn->parento, fname);
        zfn->offset = cfn->offset + off;
      }
      break;
    case 3:	/* block of zeros */
      if(blk == NULL) {
        zfn->offset = bsssize;
        bsssize += fhead[5];
        if(bsssize & 3)
          bsssize = (bsssize + 4) & ~3;
      } else {
        alignarea(blk);
        zfn->offset = addzblock(blk, fhead[5]);
      }
      break;
    default:
      error("Symbol `%s' has unknown type #%d", fname, fhead[3]);
  }
  zfn->linked = 0;
}

void link(void)
{
  struct Function *afn;

  errors = 0;
  undefined = 0;
  numprinted = numnotprinted = 0;
  /* first function must be present and in CODE */
  afn = findfunc(firstfunction, NULL);
  if(afn == NULL)
    fatal("Function `%s' (initial function) not found", firstfunction);
  if((getfhead(afn)[1] & 0xff) != CODE) {
    settfile(getstr(&bstrings, afn->objfile->filename));
    fatal("First function `%s' not in CODE segment", firstfunction);
  }

  fffn = getstr(&bstrings, findfunc(firstfunction, NULL)->objfile->filename);

  getfunc(firstfunction, NULL, firstfunction);

  settfile("(All)");

  if(errors)
    fatal("Link failed: %d undefined %s, %d serious %s",
      undefined, (undefined == 1) ? "symbol" : "symbols",
      errors-undefined, (errors-undefined == 1) ? "error": "errors");

  /* join them */
  alignarea(&bocode);
  alignarea(&bodata);
  alignarea(&bolit);

  codesize = bocode.used;
  datasize = bodata.used;
  litsize = bolit.used;

  codeoff = addtoarea(&boutput, bocode.mem, bocode.used);
  dropblock(&bocode);
  litoff  = addtoarea(&boutput, bolit.mem,  bolit.used);
  dropblock(&bolit);
  if(pagealign)
    bpagealign(&boutput);
  dataoff = addtoarea(&boutput, bodata.mem, bodata.used);
  dropblock(&bodata);
  bssoff = boutput.used;

  if(verbose) {
    fprintf(stderr, "%s: Areas are:- CODE: Size: %-8d Address: %06X\n",
              progname,
              codesize,
              codeoff + startaddr);
    fprintf(stderr, "%s:             LIT:  Size: %-8d Address: %06X\n",
              progname,
              litsize,
              litoff + startaddr);
    fprintf(stderr, "%s:             DATA: Size: %-8d Address: %06X\n",
              progname,
              datasize,
              dataoff + startaddr);
    fprintf(stderr, "%s:             BSS:  Size: %-8d Address: %06X\n",
              progname,
              bsssize,
              bssoff + startaddr);
  }
  /* link them */
  if(symfile) {
    openfile = symf = fopen(symfile, "w");
    if(!symf)
      fatal("Can't open `%s' to write link map", symfile);
  } else if(map) {
    symf = stderr;
  }

  if(symf) {
    fprintf(symf, "\nSymbol map of `%s'\n\n", outputfile);
    fprintf(symf, "CODE area begins at %06X (size %d)\n", codeoff+startaddr, codesize);
    fprintf(symf, "LIT  area begins at %06X (size %d)\n", litoff+startaddr, litsize);
    fprintf(symf, "DATA area begins at %06X (size %d)\n", dataoff+startaddr, datasize);
    fprintf(symf, "BSS  area begins at %06X (size %d)\n", bssoff+startaddr, bsssize);
    fprintf(symf, "Free area begins at %06X\n\n", bssoff+startaddr+bsssize);
    fprintf(symf, SYM_FORM2, "Symbol name", "Segment", "Scope", "Address", "File");
    fprintf(symf, SYM_FORM2, "-----------", "-------", "-----", "-------", "----");
  }

  linkfunc(firstfunction, NULL);

  if(symf) {
    printlm();
    fprintf(symf, "\n%d %s\n", numprinted, numprinted == 1 ? "symbol" : "symbols");
    if(numnotprinted)
      fprintf(symf, "\n(%d additional local %s not printed)\n", numnotprinted,
          numnotprinted == 1 ? "symbol" : "symbols");
    fprintf(symf, "\n");
  }

  if(symfile)
    fclose(symf);

  openfile = NULL;

  if(verbose)
    fprintf(stderr, "%s: Writing file `%s'\n", progname, outputfile);

  saveblock(outputfile, boutput.mem, boutput.used);
}
