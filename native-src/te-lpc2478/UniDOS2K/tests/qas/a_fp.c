#include <ctype.h>
#include <errno.h>
#include "qas.h"
#include "instr.h"
#include "ainstr.h"

//extern void storedcfe(double, unsigned int *);
//extern void storedcfp(double, unsigned int *);

/****** DCF ******/

void a_dcf(char *suf)
{
  char *tok = gettok(&restpart);
  float fv;
  double dv;
  union {
    int ivu[2];
    double dvu;
  } dtoi;
  unsigned int dcfvals[3];

  if(strlen(suf) != 1) {
    error("Illegal suffix for DCF");
    return;
  }

  if(*restpart) {
    error("Syntax: DCF<precision> <float number>");
    return;
  }
  errno = 0;
  dv = strtod(tok, NULL);
  if(errno == ERANGE)
    warning("Overflow in floating constant `%s'", tok);
  else if(errno != 0)	/* misc errs */
    warning("Bad floating constant `%s'", tok);

  switch(*suf) {
    case 's':
      fv = dv;
      addinttoarea(&odata, *(int *)(&fv));
      break;
    case 'd':
    dtoi.dvu = dv;
      addinttoarea(&odata, dtoi.ivu[0]);
      addinttoarea(&odata, dtoi.ivu[1]);
      break;
    case 'e':
//      storedcfe(dv, dcfvals);
      pwarning("DCFE constants effectively only have D precision");
      addtoarea(&odata, dcfvals, 12);
      break;
    case 'p':
//      storedcfp(dv, dcfvals);
      addtoarea(&odata, dcfvals, 12);
      break;
    default:
      error("Unknown precision `%s'", suf);
  }
}

/******* MVF/MNF/ABS/RND/SQT/LOG/LGN/EXP/SIN/COS/TAN/ASN/ACS/ATN/URD/NRM *****/

void a_fpunary(char *suf, int type)
{
  unsigned int in = 0x0e008100;
  struct fp_suffix sufb;

  if(!getsuffix_fp(FP_DATAOP, suf, &sufb))
    return;

  if(sufb.exception)
    error("Cannot specify exception flag with this instruction");

  in |= sufb.condcode | sufb.precision | sufb.rounding | (type<<20);
  in |= getregfp()<<12;
  restpart = skipw(restpart);
  if(*restpart == '#')
    in |= getfpconst(restpart+1);
  else {
    in |= getregfp();
    if(*restpart)
      error("Expected EOL found a token");
  }

  addinttoarea(&odata, in);
}

/***** ADF/MUF/SUF/RSF/DVF/RDF/POW/RPW/RMF/FML/FDV/FRD/POL/F0D/F0E/F0F ******/

void a_fpbinry(char *suf, int type)
{
  unsigned int in = 0x0e000100;
  struct fp_suffix sufb;

  if(!getsuffix_fp(FP_DATAOP, suf, &sufb))
    return;

  if(sufb.exception)
    error("Cannot specify exception flag with this instruction");

  in |= sufb.condcode | sufb.precision | sufb.rounding | (type<<20);
  in |= getregfp()<<12;
  in |= getregfp()<<16;
  restpart = skipw(restpart);
  if(*restpart == '#')
    in |= getfpconst(restpart+1);
  else {
    in |= getregfp();
    if(*restpart)
      error("Expected EOL found a token");
  }

  addinttoarea(&odata, in);
}

/****** CMF/CNF *****/

void a_fpcmp(char *suf, int type)
{
  unsigned int in = 0x0e00f110 | (type << 20);
  struct fp_suffix sufb;

  if(!getsuffix_fp(FP_STATUS, suf, &sufb))
    return;

  if(sufb.precision != S_NOTHING)
    error("Can't specify a precision with a CMF or CNF");
  if(sufb.rounding)
    error("Can't specify a rounding mode with a CMF or CNF");

  in |= sufb.condcode | sufb.exception;
  in |= getregfp()<<16;
  restpart = skipw(restpart);
  if(*restpart == '#')
    in |= getfpconst(restpart+1);
  else {
    in |= getregfp();
    if(*restpart)
      error("Expected EOL found a token");
  }
  addinttoarea(&odata, in);
}

/****** FLT/FIX/WFS/RFS/WFC/RFC *****/

void a_fpregtr(char *suf, int type)
{
  unsigned int in = 0x0e000110 | (type << 20);
  struct fp_suffix sufb;
  int ff;

  if(!getsuffix_fp(FP_RTRANS, suf, &sufb))
    return;

  if(sufb.precision == S_NOTHING) {
    if(type == 0)
      error("Must specify a precision with FLT");
    sufb.precision = PREC_S;
  }

  in |= sufb.condcode | sufb.exception | sufb.precision | sufb.rounding;

  if(type == 0) {
    ff = getregfp();	/* perhaps ? */
    in |= ff | (ff << 16);
    in |= getreg()<<12;
  } else {
    in |= getreg()<<12;
  }
  restpart = skipw(restpart);
  if(type == 1) {
    if(*restpart == '#')
      in |= getfpconst(restpart+1);
    else {
      in |= getregfp();
      if(*restpart)
        error("Expected EOL found a token");
    }
  }

  if(*restpart)
    error("Expected EOL found a token");

  addinttoarea(&odata, in);
}

/****** LDF/STF *****/

void a_fpdtran(char *suf, int type)
{
  unsigned int in = 0x0c000100 | (type << 20);
  struct fp_suffix sufb;
  int off, con;
  char *tok, *lname, *sv;
  unsigned int iv;
  struct Value *vp;

  if(!getsuffix_fp(FP_DTRANS, suf, &sufb))
    return;

  if(sufb.rounding || sufb.exception)
    error("Illegal suffix for LDF/STF");

  if(sufb.precision == S_NOTHING)
    error("Must specify a precision with LDF/STF");

  in |= sufb.condcode | sufb.precision;

  in |= getregfp()<<12;

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
        error("FP memory instructions cannot use register offset");
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
        error("FP memory instructions cannot use register offset");
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
        error("Can't make link to FP data transfer instructions");
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
