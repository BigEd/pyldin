#include <ctype.h>
#include "qas.h"
#include "instr.h"
#include "ainstr.h"

void a_cpswp(char *suf)
{
  unsigned int in = 0x01000090;
  struct suffix sufb;
  struct Value *vp;

  if(strlen(suf) == 1 || strlen(suf) == 3) {
    if(lastchar(suf) == 'b') {
      in |= 0x00400000;
      lastchar(suf) = 0;
    }
  }

  if(!getsuffix(0, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for SWP instruction");

  in |= sufb.condcode;
  in |= getreg()<<12;
  in |= getreg();
  restpart = skipw(restpart);
  if(*restpart != '[')
    error("Syntax: SWP <reg>,<reg>,[<reg>]");
  else
    restpart++;
  if(lastchar(restpart) != ']')
    error("Syntax: SWP <reg>,<reg>,[<reg>]");
  else
    lastchar(restpart) = 0;

  restpart = skipw(restpart);
  while(lastchar(restpart) == ' ') {
    lastchar(restpart) = 0;
  }

  if((vp = getvaluepos(&rns, restpart)) == NULL)
    error("Unknown register `%s'", restpart);
  else {
    if(getvaluetype(vp) != V_INT)
      error("Illegal register");
    in |= getvalueint(vp) << 16;
    addinttoarea(&odata, in);
  }
}

void a_cpdtran(char *suf, int type)
{
  unsigned int in = 0x0c000000 | (type << 20);
  struct suffix sufb;
  int off, con, n;
  char *tok, *lname, *sv;
  unsigned int iv;
  struct Value *vp;

  if(strlen(suf) == 1 || strlen(suf) == 3) {
    if(lastchar(suf) == 'l') {
      in |= 0x00400000;
      lastchar(suf) = 0;
    }
  }

  if(!getsuffix(0, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for STC/LDC instruction");

  in |= sufb.condcode;

  n = getnum();
  if(n < 0 || n > 15)
    error("Co-processor number must be in the range 0..15");
  if(n == 1)
    informational("Co-processor CP1 (FP unit) has its own mnemonics");
  in |= n<<8;

  in |= getregcp()<<12;

  restpart = skipw(restpart);
  if(*restpart == '[') {
    if(lastchar(restpart) == '!' || lastchar(restpart) == ']') {
      /* pre-indexed */
      in |= 1<<24;
      if(lastchar(restpart) == '!') {
        lastchar(restpart) = 0;
        in |= T_FLAG;
      }
      if(lastchar(restpart) == ']') {
        restpart++;
        lastchar(restpart) = 0;
      } else {
        error("Address must be enclosed in `[..]'");
        return;
      }
      in |= getreg()<<16;
      restpart = skipw(restpart);
      if(*restpart == '#' || *restpart == 0) {
        /* const offset */
        if(*restpart == 0) {
          in |= 0x800000;
          off = 0;
        } else {
          if(evaluate(restpart+1, &iv, &sv) != V_INT)
            error("Expression does not evaluate to a number");
          off = iv;
          if(off < -0x3fc || off > 0x3fc)
            error("Offset out of range");
          if(off & 3)
            error("Offset not aligned");
          if(off < 0)
            off = -off;
          else
            in |= 0x800000;
        }
        addinttoarea(&odata, in | (off >> 2));
      } else {
        error("CP memory instructions cannot use register offset");
      }
    } else {
      /* post indexed */
      tok = gettokspc(&restpart);
      restpart = skipw(restpart);
      if(*restpart == 0) {
        error("Must supply an offset");
        return;
      }
      if((*tok != '[') || (lastchar(tok) != ']')) {
        error("Register must be contained within `[..]'");
        return;
      }
      tok++;
      lastchar(tok) = 0;
      vp = getvaluepos(&rns, tok);
      if(vp == NULL) {
        error("Unknown register `%s'", tok);
        return;
      }
      in |= getvalueint(vp)<<16;

      if(*restpart == '#') {
        /* const offset */
        if(evaluate(restpart+1, &iv, &sv) != V_INT)
          error("Expression does not evaluate to a number");
        off = iv;
        if(off < -0x3fc || off > 0x3fc)
          error("Offset out of range");
        if(off & 3)
          error("Offset not aligned");
        if(off < 0)
          off = -off;
        else
          in |= 0x800000;
        addinttoarea(&odata, in | (off >> 2));
      } else {
        error("CP memory instructions cannot use register offset");
      }
    }
  } else {
    /* pc relative */
    in |= (1<<24) | (15<<16);
    lname = labeloffset(restpart, &off);
    if(lname) {
      if(off < 0 || off > 0x800)
        error("Offset out of range");
      if(*lname == '.') {
        if((vp = getvaluepos(&llabels, lname)) == NULL) {
          setbackpatch(lname, in | off, BP_FPDTRANS);
          addinttoarea(&odata, 0);
        } else {
          if(getvaluetype(vp) != V_INT)
            error("Illegal local label");
          con = ((getvalueint(vp)+off)-odata.used)-8;
          if(con < -0x3fc || con > 0x3fc)
            error("Offset out of range");
          if(con & 3)
            error("Offset not aligned");
          if(con < 0)
            con = -con;
          else
            in |= 1<<23;
          addinttoarea(&odata, in | ((con>>2) & 0xff));
        }
      } else {
        error("Can't make link to CP data transfer instructions");
      }
    } else {
      if(evaluate(restpart, &iv, &sv) == V_INT) {
        off = iv-8;
        if(off < -0x3fc || off > 0x3fc)
          error("Offset out of range");
        if(off < 0)
          off = -off;
        else
          in |= 1<<23;
        addinttoarea(&odata, in | ((off>>2) & 0xff));
      } else
        error("Expression does not evaluate to int");
    }
  }
}

void a_cpoper(char *suf, int type)	/* 0=cdp, 1=mrc, 2=mcr */
{
  unsigned int in = 0x0e000000;
  struct suffix sufb;
  int n;

  if(!getsuffix(0, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for CP instruction");

  in |= sufb.condcode;
  n = getnum();
  if(n < 0 || n > 15)
    error("Co-processor number must be in the range 0..15");
  if(n == 1)
    informational("Co-processor CP1 (FP unit) has its own mnemonics");
  in |= n<<8;
  n = getnum();
  if(n < 0 || n > 15)
    error("Operation must be in the range 0..15");
  if(n&1)
    warning("CP operation not even");
  in |= n<<20;
  switch(type) {
    case 0:
      in |= getregcp()<<12;
      break;
    case 1:
      in |= 0x00100010;
      in |= getreg()<<12;
      break;
    case 2:
      in |= 0x00000010;
      in |= getreg()<<12;
      break;
    default:
      fatal("Internal error: Unknown CP instruction");
  }
  in |= getregcp()<<16;
  in |= getregcp();
  restpart = skipw(restpart);
  if(*restpart) {
    n = getnum();
    if(n < 0 || n > 7)
      error("Info field must be in the range 0..7");
    in |= n<<5;
  }
  if(*restpart)
    error("Expected EOL found a token");
  addinttoarea(&odata, in);
}
