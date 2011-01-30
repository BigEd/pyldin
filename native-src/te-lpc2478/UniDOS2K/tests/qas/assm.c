#include "qas.h"

struct BackPatch {
  struct BackPatch *next, *prev;
  int line;
  enum BPType type;
  int value;
  int where;
  char *label;
};

char *creator, *procos;
int numfuncs, numlinkis;
int funcheadsoff = 0;

static struct BackPatch *bplist = NULL;

int PARENTO;

static void decomment(char *m)
{
  char pc = 0, c = 0, *p = m;
  int indq = 0, insq = 0;

  for(;;) {
    pc = c;
    c = *p++;
    if(c == '\0')		/* return if eol */
      return;
    if((c == '\'') && !indq) {	/* if single-quote not in double-quote, either; */
      if(!insq) {
        insq = 1;		/* begin single-quote */
      } else {
        if(pc != '\\')		/* or, if not '\'', then end single-quote */
          insq = 0;
      }
    }
    if((c == '"') && !insq) {	/* if double-quote not in single-quote, either; */
      if(!indq) {
        indq = 1;		/* begin double-quote */
      } else {
        if(pc != '\\')		/* or, if not '\'', then end double-quote */
          indq = 0;
      }
    }
    if((c == ';') && !indq && !insq) {	/* if semicolon not in either quote,	*/
      *(p-1) = '\0';			/* end line and return.			*/
      return;
    }
  }
}

static void stripw(char *m)
{
  char *p = m + strlen(m) - 1;

  while(*p == ' ' || *p == '\t') {
    *p = 0;
    p--;
  }
}

void assemble(char *outname)
{
  char *aline;
  int dataoffset, stringsoffset, linkisoffset;
  int datalen, stringslen, linkislen, oldlineno;
  struct export *e, *eo;

  numfuncs = 0;
  numlinkis = 0;
  temptype = -2;
  if(src == NULL)
    return;
  if(verbose)
    fprintf(stderr, "\n%s: Assembling file `%s'\n", progname, src->filename);
  errors = 0;
  makearea(&oheaders);
  makearea(&ostrings);
  makearea(&odata);
  makearea(&olinkis);

  PARENTO = addonestrtoarea(outname);

  /* build header */
  addinttoarea(&oheaders, 0x6a624f51);	/* "QObj" */
  addinttoarea(&oheaders, -1);		/* creator string */
  addinttoarea(&oheaders, -1);		/* num funcs */
  addinttoarea(&oheaders, 60);		/* address func headers */
  addinttoarea(&oheaders, -1);		/* procos */
  addinttoarea(&oheaders, 0);		/* reserved */
  addinttoarea(&oheaders, 2);		/* obj format version */
  addinttoarea(&oheaders, 0);		/* address code/data */
  addinttoarea(&oheaders, 0);		/* length code/data */
  addinttoarea(&oheaders, 0);		/* address strings */
  addinttoarea(&oheaders, 0);		/* length strings */
  addinttoarea(&oheaders, 0);		/* address linkis */
  addinttoarea(&oheaders, 0);		/* length linkis */
  addinttoarea(&oheaders, 0);		/* address debug info */
  addinttoarea(&oheaders, 0);		/* length debug info */
  strcpy(creator, "qas RISC OS ARM Assembler vsn "VERS" © Stu Smith ["__DATE__"]");
  strcpy(procos, "ARM/RISCOS");

  funcheadsoff = oheaders.used;

  /* assemble line-by-line */
  parseinit();
  clearvalues(&llabels);
  clearvalues(&rns);
  clearvalues(&fns);
  clearvalues(&cns);
  clearvalues(&sets);

  while(src) {
    aline = getline();
    decomment(aline);
    stripw(aline);
    if(*aline) {
      assmline(aline);
    }
  }
  endprevfunc();
  placeint(&oheaders, addonestrtoarea(creator), 4);
  placeint(&oheaders, addonestrtoarea(procos), 16);
  placeint(&oheaders, numfuncs, 8);

  /* concat headers-data-strings */
  alignarea(&oheaders);
  alignarea(&odata);
  alignarea(&ostrings);
  dataoffset = addtoarea(&oheaders, odata.mem, odata.used);
  datalen = odata.used;
  droparea(&odata);
  alignarea(&oheaders);
  linkisoffset = addtoarea(&oheaders, olinkis.mem, olinkis.used);
  linkislen = olinkis.used;
  droparea(&olinkis);
  alignarea(&oheaders);
  stringsoffset = addtoarea(&oheaders, ostrings.mem, ostrings.used);
  stringslen = ostrings.used;
  droparea(&ostrings);
  alignarea(&oheaders);

  if(numfuncs == 0)
    error("No symbols defined");

  placeint(&oheaders, dataoffset,    28);
  placeint(&oheaders, datalen,       32);
  placeint(&oheaders, stringsoffset, 36);
  placeint(&oheaders, stringslen,    40);
  placeint(&oheaders, linkisoffset,  44);
  placeint(&oheaders, linkislen,     48);
  placeint(&oheaders, 0,             52);
  placeint(&oheaders, 0,             56);

  e = exportlist;
  while(e) {
    oldlineno = lineno;
    lineno = e->line;
    error("Can't export symbol `%s': not defined", e->symname);
    lineno = oldlineno;
    freemem(e->symname);
    eo = e->next;
    freemem(e);
    e = eo;
  }
  exportlist = NULL;

  /* save object file */
#ifndef DEBUG
  if(errors == 0)
#endif
    saveblock(outname, oheaders.mem, oheaders.used);
  droparea(&oheaders);
}

void setbackpatch(char *labelname, int value, enum BPType btype)
{
  struct BackPatch *zbp;

  /* add new entry to backpatch list */
  zbp = getmem(sizeof(struct BackPatch));
  zbp->where = odata.used;
  zbp->line = lineno;
  zbp->value = value;
  zbp->type = btype;
  zbp->label = getmem(strlen(labelname)+1);
  strcpy(zbp->label, labelname);
  zbp->prev = NULL;
  if(bplist)
    bplist->prev = zbp;
  zbp->next = bplist;
  bplist = zbp;
}

void backpatchlabel(char *labelname)
{
  struct BackPatch *bp, *nbp, *pbp, *zbp;
  int off, vv, vvv, reg, in;
  unsigned int ui;
  struct Value *vp = getvaluepos(&llabels, labelname);

  vvv = getvalueint(vp);
  bp = bplist;
  while(bp) {
    if(!strcmp(bp->label, labelname)) {
      switch(bp->type) {
        case BP_ADR:
          in  = getint(&odata, bp->where);
          reg = getint(&odata, bp->where + 4);
          vv = vvv + bp->value;;
          ui = (vv - bp->where)-8;
          if((vv - bp->where)-8 < 0) {
            in |= 2<<21;	/* sub */
            ui = (unsigned int) (- (int) ui);
          } else
            in |= 4<<21;	/* add */
          placeint(&odata, in | constval(&ui) | (15<<16), bp->where);
          placeint(&odata, in | constval(&ui) | (reg<<16), bp->where + 4);
          placeint(&odata, in | constval(&ui) | (reg<<16), bp->where + 8);
          break;
        case BP_BRANCH:
          off = bp->value & 0x00ffffff;
          bp->value &= 0xff000000;
          vv = vvv + off;
          if((vv - bp->where)&3)
            error("Offset must be aligned");
          placeint(&odata, bp->value | ((((vv - bp->where)-8)>>2) & 0x00ffffff), bp->where);
          break;
        case BP_DTRANS:
          off = bp->value & 0xfff;
          in = bp->value & 0xfffff000;
          vv = vvv + off;
          vv = (vv - bp->where)-8;
          if((vv < -4095) || (vv > 4095))
            error("Offset out of range");
          if(vv < 0)
            vv = -vv;
          else
            in |= 1<<23;
          placeint(&odata, in | (vv&0xfff), bp->where);
          break;
        case BP_FPDTRANS:
          off = bp->value & 0xff;
          in = bp->value & 0xffffff00;
          vv = vvv + off;
          vv = (vv - bp->where)-8;
          if((vv < -0x3fc) || (vv > 0x3fc))
            error("Offset out of range");
          if(vv & 3)
            error("Offset not aligned");
          if(vv < 0)
            vv = -vv;
          else
            in |= 1<<23;
          placeint(&odata, in | ((vv >> 2) & 0xff), bp->where);
          break;
        case BP_DFNL:
          addinttoarea(&olinkis, bp->where - curfuncstart);
          addinttoarea(&olinkis, addonestrtoarea(curfuncname));
          addinttoarea(&olinkis, bp->value + vvv - curfuncstart);
          numlinkis++;
          break;
        default:
          fatal("Internal error: Unknown backpatch type");
      }

      freemem(bp->label);
      zbp = bp;
      nbp = bp->next;
      pbp = bp->prev;
      freemem(bp);
      if(zbp == bplist)
        bplist = nbp;
      if(pbp)
        pbp->next = nbp;
      if(nbp)
        nbp->prev = pbp;
      bp = nbp;
    } else
      bp = bp->next;
  }
}

int backpatchfend(void)
{
  struct BackPatch *bp, *zbp;
  int ok = 1, oldlineno;

  /* clear list */
  bp = bplist;
  if(bplist)
    ok = 0;
  while(bp) {
    oldlineno = lineno;		/* save lineno, replace with line at which */
    lineno = bp->line;		/* llabel is used (not when it is found missing) */
    error("Local label `%s' not defined", bp->label);
    lineno = oldlineno;
    freemem(bp->label);
    zbp = bp;
    bp = bp->next;
    freemem(zbp);
  }
  bplist = NULL;
  return ok;
}
