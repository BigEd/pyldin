#include "qas.h"


static int hascode, curfunchead, hasblock;
static enum Segment curseg;
char *curfuncname = NULL;
char *restpart;
int curfuncstart, infunc;
int temptype; /* 0-15 reg, -1 stack, -2 none */

struct export *exportlist = NULL;

static void export(char *sname)
{
  int i;
  struct export *e;

  /* check if symbol is already defined */
  for(i = 0; i < numfuncs; i++) {
    if(!strcmp(sname, ostrings.mem + getint(&oheaders, i*FUNCHEADSZ + funcheadsoff))) {
      placeint(&oheaders,
      getint(&oheaders, i*FUNCHEADSZ + funcheadsoff + 4) & 0xff | GLOBAL,
      i*FUNCHEADSZ + funcheadsoff + 4);
      return;
    }
  }

  /* check if sname is on list */
  e = exportlist;
  while(e) {
    if(!strcmp(e->symname, sname))
      return;
    e = e->next;
  }

  /* symbol not yet defined, add to export list */
  e = getmem(sizeof(struct export));
  e->symname = getmem(strlen(sname)+1);
  e->line = lineno;
  strcpy(e->symname, sname);
  e->next = exportlist;
  exportlist = e;
}

static void makeoffsetfunc(char *name)
{
  int sc = STATIC;
  struct export *e, **ep;

  /* check to see if function is on export list */
  e = exportlist;
  ep = &exportlist;
  while(e) {
    if(!strcmp(name, e->symname)) {
      sc = GLOBAL;
      *ep = e->next;
      freemem(e->symname);
      freemem(e);
      break;
    }
    ep = &(e->next);
    e = e->next;
  }

  addinttoarea(&oheaders, addonestrtoarea(name));
  addinttoarea(&oheaders, curseg | sc);	/* inherited from parent */
  addinttoarea(&oheaders, PARENTO);	/* parent */
  addinttoarea(&oheaders, 2);		/* offset function */
  addinttoarea(&oheaders, addonestrtoarea(curfuncname));
  addinttoarea(&oheaders, odata.used - curfuncstart);	/* offset */
  addinttoarea(&oheaders, 0);		/* unused */
  addinttoarea(&oheaders, 0);		/* junk in num linkis */
  addinttoarea(&oheaders, 0);		/* no debug info */
  numfuncs++;
}

void parseinit(void)
{
  if(!curfuncname)
    curfuncname = getmem(256);
  infunc = 0;
  curseg = CODE;
  curfuncstart = -1;
  curfunchead = -1;
}

static void twosections(char *l, char **lp, char **rp)
{
  char *spc;

  *lp = NULL;
  *rp = NULL;
  if(*l != ' ')	{	/* line won't be null */
    if(*l == '|') {
      *lp = l+1;
      spc = strchr(l+1, '|');
      if(spc == NULL)
        error("Unmatched `|'");
    } else {
      *lp = l;
      spc = strchr(l, ' ');
    }
  } else
    spc = strchr(l, ' ');

  if(spc) {
    *spc++ = '\0';	/* ends label part if set */
    while(*spc == ' ')
      spc++;
    if(*spc)
      *rp = spc;
  }
}

void endprevfunc(void)
{
  infunc = 0;
  if(numfuncs != 0) {
    if(odata.used - curfuncstart == 0 && hasblock == 0)
      error("Symbol `%s' has zero length", curfuncname);
    if(!hasblock)
      placeint(&oheaders, odata.used - curfuncstart, curfunchead+20);
    placeint(&oheaders, numlinkis, curfunchead+28);
    backpatchfend();
  }
}

static void parseswitch(char *name)
{
  char *toks, *sv, *strv;
  unsigned int iv, intv;
  int ty;
  struct Value *pos;

  if(!strcmp(name, "CREATOR:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(evaluate(toks, &iv, &sv) == V_STRING) {
        if(strlen(sv) > 255) {
          error("String too long");
          return;
        }
        strcpy(creator, sv);
        if(*restpart == 0)
          return;
      }
    }
    error("Syntax: CREATOR: \"<string>\"");
    return;
  }

  if(!strcmp(name, "MSG:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(evaluate(toks, &iv, &sv) == V_STRING) {
        informational("MSG: %s", sv);
        if(*restpart == 0)
          return;
      }
    }
    error("Syntax: MSG: \"<string>\"");
    return;
  }


  if(!strcmp(name, "PLATFORM:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(evaluate(toks, &iv, &sv) == V_STRING) {
        if(strlen(sv) > 255) {
          error("String too long");
          return;
        }
        strcpy(procos, sv);
        if(*restpart == 0)
          return;
      }
    }
    error("Syntax: PLATFORM: \"<string>\"");
    return;
  }

  if(!strcmp(name, "GET:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(evaluate(toks, &iv, &sv) == V_STRING) {
        setSource(sv);
        if(*restpart == 0)
          return;
      }
    }
    error("Syntax: GET: <filename>");
    return;
  }

  if(!strcmp(name, "RN:")) {
    toks = gettok(&restpart);
    if(toks) {
      strv = toks;
      if(*restpart) {
        if(evaluate(restpart, &iv, &sv) == V_INT) {
          intv = iv;
          if(intv < 0 || intv > 15) {
            error("Register values must be in the range 0-15");
            return;
          }
          addintvalue(&rns, strv, intv);
          return;
        }
      }
    }
    error("Syntax: RN: <name> <number>");
    return;
  }

  if(!strcmp(name, "FN:")) {
    toks = gettok(&restpart);
    if(toks) {
      strv = toks;
      if(*restpart) {
        if(evaluate(restpart, &iv, &sv) == V_INT) {
          intv = iv;
          if(intv < 0 || intv > 7) {
            error("FP Register values must be in the range 0-7");
            return;
          }
          addintvalue(&fns, strv, intv);
          return;
        }
      }
    }
    error("Syntax: FN: <name> <number>");
    return;
  }

  if(!strcmp(name, "CN:")) {
    toks = gettok(&restpart);
    if(toks) {
      strv = toks;
      if(*restpart) {
        if(evaluate(restpart, &iv, &sv) == V_INT) {
          intv = iv;
          if(intv < 0 || intv > 15) {
            error("CP Register values must be in the range 0-15");
            return;
          }
          addintvalue(&cns, strv, intv);
          return;
        }
      }
    }
    error("Syntax: FN: <name> <number>");
    return;
  }

  if(!strcmp(name, "LINE:")) {
    if(*restpart) {
      if(evaluate(restpart, &iv, &sv) == V_INT) {
        lineno = iv;
        if(!src) {
          fatal("Internal error: src is NULL");
          return;
        }
        src->line = iv;
        return;
      }
    }
    error("Syntax: LINE: <number>");
    return;
  }

  if(!strcmp(name, "EXPORT:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(*toks == '|') {
        toks++;
        if(lastchar(toks) != '|')
          error("Unmatched `|'");
        lastchar(toks) = 0;
      }
      export(toks);
      return;
    } else {
      error("Syntax: EXPORT: symbolname");
      return;
    }
    if(*restpart == 0)
      return;
    error("Syntax: EXPORT: symbolname");
    return;
  }

  if(!strcmp(name, "SEGMENT:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(!strcmp(toks, "CODE")) {
        curseg = CODE;
        endprevfunc();
      } else if(!strcmp(toks, "DATA")) {
        curseg = DATA;
        endprevfunc();
      } else if(!strcmp(toks, "BSS")) {
        curseg = BSS;
        endprevfunc();
      } else if(!strcmp(toks, "LIT")) {
        curseg = LIT;
        endprevfunc();
      } else {
        error("Syntax: SEGMENT: CODE|DATA|BSS|LIT");
        return;
      }
      if(*restpart == 0)
        return;
    }
    error("Syntax: SEGMENT: CODE|DATA|BSS|LIT");
    return;
  }

  if(!strcmp(name, "SET:")) {
    toks = gettok(&restpart);
    if(toks) {
      strv = toks;
      if(*restpart) {
        ty = evaluate(restpart, &iv, &sv);
        if(ty == V_INT) {
          addintvalue(&sets, strv, iv);
          return;
        } else if(ty == V_STRING) {
          addstrvalue(&sets, strv, sv);
          return;
        }
      }
    }
    error("Syntax: SET: <name> <value>");
    return;
  }

  if(!strcmp(name, "WITHIN:")) {
    toks = gettok(&restpart);
    if(toks) {
      /* symbol name may be quoted */
      if(*toks == '|') {
        if(lastchar(toks) != '|')
          error("Unmatched `|'");
        else {
          toks++;
          lastchar(toks) = 0;
        }
      }
      makeoffsetfunc(toks);
      if(*restpart == 0)
        return;
    }
    error("Syntax: WITHIN: <symbol name>");
    return;
  }

  if(!strcmp(name, "BLOCK:")) {
    if(*restpart) {
      if(evaluate(restpart, &iv, &sv) == V_INT) {
        if(hascode) {
          error("BLOCK: must comprise the whole of a symbol");
          return;
        }
        if((int)iv < 0) {
          error("Argument to BLOCK: must be positive or zero");
          return;
        }
        placeint(&oheaders, 3, curfunchead+12);		/* change func. type */
        placeint(&oheaders, 0, curfunchead+16);		/* unused */
        placeint(&oheaders, iv, curfunchead+20);	/* size of zero block */
        placeint(&oheaders, 0, curfunchead+24);		/* unused */
        hasblock = 1;
        endprevfunc();			/* BLOCK: ends function */
        return;
      }
    }
    error("Syntax: BLOCK: <number>");
    return;
  }

  if(!strcmp(name, "TEMP:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(!strcmp(toks, "NONE")) {
        temptype = -2;
      } else if(!strcmp(toks, "STACK")) {
        temptype = -1;
      } else {
        pos = getvaluepos(&rns, toks);
        if(pos == NULL) {
          error("Unknown register `%s'", toks);
          return;
        }
        temptype = getvalueint(pos);
        if(temptype == 15)
          error("PC may not be temporary register");
      }
      if(*restpart == 0)
        return;
    }
    error("Syntax: TEMP: register|STACK|NONE");
    return;
  }

  if(!strcmp(name, "ERROR:")) {
    toks = gettok(&restpart);
    if(toks) {
      if(evaluate(toks, &iv, &sv) == V_STRING) {
        error("ERROR: %s", sv);
        if(*restpart == 0)
          return;
      }
    }
    error("Syntax: ERROR: \"<string>\"");
    return;
  }

  error("Unknown switch `%s'", name);
}

static void newfunction(char *name)
{
  int i;
  struct export *e, **ep;

  endprevfunc();

  /* check no duplicate symbol */
  for(i = 0; i < numfuncs; i++) {
    if(!strcmp(name, ostrings.mem + getint(&oheaders, i*36 + funcheadsoff)))
      error("Symbol `%s' already defined", name);
  }

  clearvalues(&llabels);
  strcpy(curfuncname, name);
  alignarea(&odata);
  curfuncstart = odata.used;
  curfunchead =
  addinttoarea(&oheaders, addonestrtoarea(name));
  addinttoarea(&oheaders, curseg | STATIC);
  addinttoarea(&oheaders, PARENTO);
  addinttoarea(&oheaders, 1);		/* must be standard func (or block)*/
  addinttoarea(&oheaders, odata.used);
  addinttoarea(&oheaders, 0);		/* junk in length */
  addinttoarea(&oheaders, olinkis.used);
  addinttoarea(&oheaders, 0);		/* junk in num linkis */
  addinttoarea(&oheaders, 0);		/* no debug info */
  infunc = 1;
  hascode = 0;
  hasblock = 0;
  numfuncs++;
  numlinkis = 0;

  /* check to see if function is on export list */
  e = exportlist;
  ep = &exportlist;
  while(e) {
    if(!strcmp(name, e->symname)) {
      placeint(&oheaders,
               getint(&oheaders, curfunchead+4) & 0xff | GLOBAL,
               curfunchead+4);
      *ep = e->next;
      freemem(e->symname);
      freemem(e);
      break;
    }
    ep = &(e->next);
    e = e->next;
  }
}

static void skiprest(void)
{
  while(*restpart != 0)
    gettok(&restpart);
}

void assmline(char *line)
{
  char *labelpart, *tok;;

  twosections(line, &labelpart, &restpart);
  if(labelpart) {
    /* label may end with ':' */
    if(lastchar(labelpart) == ':')
      lastchar(labelpart) = 0;
    /* label may be quoted */
    if(*labelpart == '|') {
      if(lastchar(labelpart) != '|')
        error("Unmatched `|'");
      else {
        labelpart++;
        lastchar(labelpart) = 0;
      }
    }
    if(*labelpart != '.') {	/* ie symbol */
      newfunction(labelpart);
    } else {			/* local label */
      if(!infunc) {
        error("Local label `%s' found outside function", labelpart);
      } else {
        addintvalue(&llabels, labelpart, odata.used);
        backpatchlabel(labelpart);
      }
    }
  }
  if(restpart) {
    for(;;) {
      tok = gettok(&restpart);
      if(tok) {
        if(lastchar(tok) == ':') {
          parseswitch(tok);
          skiprest();
        } else {
          if(curseg == BSS)
            error("Can't place code in BSS segment");
          else {
            parseinstr(tok);
            hascode = 1;
            skiprest();
          }
        }
      } else
        break;
    }
  }
}
