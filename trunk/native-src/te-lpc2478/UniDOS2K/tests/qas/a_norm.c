#include <ctype.h>
#include "qas.h"
#include "instr.h"
#include "ainstr.h"

/***** SWI ******/

void a_swi(char *suf)
{
  unsigned int iv;
  char *sv;
  struct suffix sufb;

  if(!getsuffix(SWI, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for SWI instruction");

  if(*restpart == NULL) {
    error("Syntax: SWI <number>");
    return;
  }

  if(evaluate(restpart, &iv, &sv) != V_INT) {
    error("Expression does not evaluate to a number");
    return;
  }

  if((iv < 0) || (iv > 0x00ffffff)) {
    error("SWI number out of range");
    return;
  }

  addinttoarea(&odata, sufb.condcode | 0x0f000000 | iv);
}

/***** MUL/MLA *****/

void a_mul(char *suf, int acc)
{
  unsigned int in = 0x90;
  struct suffix sufb;
  int rn, rd = 0, rs, rm, rtemp, sreg;

  if(!getsuffix(MULT, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.p_flag)
    error("Illegal option for MUL/MLA instruction");

  in |= (sufb.s_flag) | (acc << 21) | (sufb.condcode);

  rn = getreg();
  rm = getreg();
  rs = getreg();
  if(acc)
    rd = getreg();

  if(*restpart)
    if(acc)
      error("Syntax MLA Rn, Rm, Rs, Rd");
    else
      error("Syntax MUL Rn, Rm, Rs");

  if(rn == 15)
    warning("Destination may not be r15");
  if(rn == rm) {
    pwarning("Destination may not be the same as first source");
    rtemp = rm;
    rm = rs;
    rs = rtemp;
    if(rn == rm) {
      if(temptype == -2) {
        warning("Can't successfully swap registers");
        /* ** illegal ** */
        addinttoarea(&odata, in | (rn << 16) | (rd << 12) | (rs << 8) | rm);
      } else if(temptype == -1) {
        sreg = 0;
        while(sreg == rn || sreg == rd)
          sreg++;
        /* stmfd sp!,{sreg}	*/
        addinttoarea(&odata, 0x092d0000 | sufb.condcode | (1<<sreg));
        /* mov sreg, rm	*/
        addinttoarea(&odata, 0x01a00000 | sufb.condcode | (sreg << 12) | rm);
        /* mul rn, sreg, rs, rd */
        addinttoarea(&odata, in | (rn << 16) | (rd << 12) | (rs << 8) | sreg);
        /* ldmfd sp!,{sreg}	*/
        addinttoarea(&odata, 0x08bd0000 | sufb.condcode | (1<<sreg));
      } else {
        /* mov <temp>, rm */
        addinttoarea(&odata, 0x01a00000 | sufb.condcode | (temptype << 12) | rm);
        /* mul rn, <temp>, rs, rd */
        addinttoarea(&odata, in | (rn << 16) | (rd << 12) | (rs << 8) | temptype);
      }
    } else {
      addinttoarea(&odata, in | (rn << 16) | (rd << 12) | (rs << 8) | rm);
    }
  } else {
    addinttoarea(&odata, in | (rn << 16) | (rd << 12) | (rs << 8) | rm);
  }
}

/****** AND/EOR/XOR/SUB/RSB/ADD/ADC/SBC/RSC/TST/TEQ/CMP/CMN/ORR/MOV/BIV/MVN *******/

static void assmconst(int reg, unsigned int c, unsigned int cc)
{
  unsigned int in = 0x3a00000;

  /* check if we can mvn it in one */
  if(oneconst(~c)) {
    c = ~c;
    in = 0x3e00000 | constval(&c) | (reg << 12);
    if(c)
      fatal("Internal: oneconst/constval corrupt");
    addinttoarea(&odata, cc | in);
  } else {
    in |= constval(&c) | (reg << 12);
    addinttoarea(&odata, cc | in);
    in = 0x3800000 | (reg << 12) | (reg << 16);
    if(reg == 15 && c)
      error("Attempt to compute large value into PC");
    while(c) {
      addinttoarea(&odata, cc | in | constval(&c));
    }
  }
}

void a_datapro(char *suf, int which)
{
  unsigned int in = 0;
  struct suffix sufb;
  char *sv;
  unsigned int iv = 0;
  int reg, sreg, which2, iiv, reg2;
  union {
    unsigned int u;
    int i;
  } ui;

  if(which == 0xff)
    in |= (0x4 << 21);
  else
    in |= (which << 21);
  if(!getsuffix(DATAPRO, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag)
    error("Illegal option for data process instruction");
  if(which >= 8 && which <= 11) {	/* TST, TEQ, CMP, CMN */
    in |= S_FLAG;
    if(sufb.s_flag)
      informational("Use of S flag with comparison instruction");
    in |= sufb.p_flag;
  } else {				/* others */
    in |= sufb.s_flag;
    if(sufb.p_flag)
      error("Can't use P flag with this instruction");
  }

  if(which >= 8 && which <= 11) {		/* TST, TEQ, CMP, CMN */
    reg = getreg();
    in |= (reg << 16);
    restpart = skipw(restpart);
    if(*restpart == '#') {	/* const */
      if(evaluate(restpart+1, &iv, &sv) != V_INT)
        error("Expression does not evaluate to int");
      /* value is iv */
      ui.u = iv;
      iiv = ui.i;
      if(iiv < 0 && iv != 0x80000000) {
        if(which == 10) {	/* CMP */
          which = 11;		/* cmn */
          iv = (unsigned int)(-iiv);
          in &= ~(0xf << 21);	/* clear which */
          in |= (which<<21);
        } else if(which == 11) {	/* CMN */
          which = 10;		/* cmp */
          iv = (unsigned int)(-iiv);
          in &= ~(0xf << 21);	/* clear which */
          in |= (which<<21);
        }
      }
      if(oneconst(iv)) {
        in |= constval(&iv);
        if(iv)
          fatal("Internal: oneconst/constval corrupt");
        addinttoarea(&odata, sufb.condcode | in | (1<<25));
      } else {
        if(temptype == -2)
          error("Bad immediate constant - no temporary register set");
        else if((temptype == -1) || (reg == temptype)) {
          if(sufb.condcode != AL)
            error("Bad immediate constant - can't stack with this c/code");
          else {
            /* we will use either of r0 or r1 */
            if(reg == 0)
              sreg = 1;
            else
              sreg = 0;
            /* stmfd sp!,{sreg}	*/
            addinttoarea(&odata, 0x092d0000 | sufb.condcode | (1<<sreg));
            /* mov sreg, #iv	*/
            assmconst(sreg, iv, sufb.condcode);
            /* cmp reg, sreg	*/
            addinttoarea(&odata, in | sufb.condcode | sreg);
            /* ldmfd sp!,{sreg}	*/
            addinttoarea(&odata, 0x08bd0000 | sufb.condcode | (1<<sreg));
          }
        } else {
          assmconst(temptype, iv, sufb.condcode);
          in |= temptype;
          addinttoarea(&odata, sufb.condcode | in);
        }
      }
    } else {
      in |= getregshift(1);
      addinttoarea(&odata, sufb.condcode | in);
    }
  } else if(which == 13 || which == 15) {	/* MOV, MVN */
    reg = getreg();
    in |= (reg << 12);
    restpart = skipw(restpart);
    if(*restpart == '#') {	/* const */
      if(evaluate(restpart+1, &iv, &sv) != V_INT)
        error("Expression does not evaluate to int");
      /* value is iv */
      in |= 1<<25;
      if(which == 0xf) {	/* mvn */
        in &= ~(1<<22);		/* change to mov */
        iv = ~iv;
      }
      if(sufb.s_flag && !oneconst(iv))
        error("S flag used with large constant");
      assmconst(reg, iv, sufb.condcode | sufb.s_flag);
    } else {
      in |= getregshift(1);
      addinttoarea(&odata, sufb.condcode | in);
    }
  } else {					/* others */
    reg = getreg();
    in |= (reg << 12);
    reg2 = getreg();
    restpart = skipw(restpart);
    if(*restpart == '#') {	/* const */
      if(evaluate(restpart+1, &iv, &sv) != V_INT)
        error("Expression does not evaluate to int");
      /* value is iv */

      ui.u = iv;
      iiv = ui.i;
      if(iiv < 0 && iv != 0x80000000) {
        switch(which) {
          case 2:	/* SUB */
            iv = (unsigned int)(-iiv);
            which = 4;	/* add */
            in &= ~(0xf << 21);	/* clear which */
            in |= (which<<21);
            pwarning("Negative value used as immediate constant");
            break;
          case 3:	/* RSB */
            error("RSB used with negative immediate constant");
            break;
          case 4:	/* ADD */
            iv = (unsigned int)(-iiv);
            which = 2;	/* sub */
            in &= ~(0xf << 21);	/* clear which */
            in |= (which<<21);
            pwarning("Negative value used as immediate constant");
            break;
          case 5:	/* ADC */
            error("ADC used with negative immediate constant");
            break;
          case 6:	/* SBC */
            error("SBC used with negative immediate constant");
            break;
          case 7:	/* RSC */
            error("RSC used with negative immediate constant");
            break;
          case 0xff:	/* SET */
            iv = (unsigned int)(-iiv);
            which = 2;	/* sub */
            in &= ~(0xf << 21);	/* clear which */
            in |= (which<<21);
            break;
        }
      }

      if(which == 0xff)
        which = 0x4;

      in |= 1<<25;
      if(oneconst(iv)) {
        addinttoarea(&odata, sufb.condcode | in | constval(&iv) | (reg2 << 16));
      } else {
        in &= ~(0xf << 21);	/* clear which */
        which2 = which;
        switch(which) {
          case 0:		/* AND */
            iv = ~iv;
            which = 14;	/* bic */
            which2 = 14;
            break;
          case 3:		/* RSB */
            which2 = 4;	/* add */
            break;
          case 5:		/* ADC */
            which2 = 4;	/* add */
            break;
          case 6:		/* SBC */
            which2 = 2;	/* sub */
            break;
          case 7:		/* RSC */
            which2 = 4;	/* add */
            break;
        }
        addinttoarea(&odata,
           sufb.condcode | in | constval(&iv) | (which << 21) | (reg2 << 16));
        while(iv) {
          addinttoarea(&odata,
             sufb.condcode | in | constval(&iv) | (which2 << 21) | (reg << 16));
        }
      }
    } else {
      in |= getregshift(1);
      addinttoarea(&odata, sufb.condcode | in | (reg2 << 16));
    }
  }
}

/***** DCB/DCW/DCD/DBB/DBW/DBD/DFL *****/

void a_declare(char *suf, enum Declare_Type type)
{
  char *tok, *sv, *lname;
  unsigned int iv, nn;
  int num;
  enum ValueType vt;
  int off, at, ll;
  struct Value *vp;
  union {
    unsigned int u;
    int i;
  } ui;

  if(*suf)
    error("Data declarations may not have options");

  switch(type) {
    case DT_DFL:
    case DT_DWL:
      tok = gettokspc(&restpart);
      while(tok) {
        lname = labeloffset(tok, &off);
        if(lname) {
          /* add link */
          if(*lname == '.') {
            error("Can't declare length of local labels");
          } else {
            /* linki either DFN or DWF */
            at = addinttoarea(&odata, 0x0cff0000) - curfuncstart;
            if(at & 0xff000000)
              fatal("a_declare(): illegal `at'");
            if(type == DT_DWL)
              at |= 0x01000000;
            addinttoarea(&olinkis, at);
            addinttoarea(&olinkis, addonestrtoarea(lname));
            addinttoarea(&olinkis, off);
            numlinkis++;
          }
        } else {
          error("Syntax: DFL <symbol> [ {+|-} <expression> ] [, ...]");
        }
        tok = gettokspc(&restpart);
      }
      break;
    case DT_DCB:
      tok = gettokspc(&restpart);
      while(tok) {
        vt = evaluate(tok, &iv, &sv);
        ui.u = iv;
        if(vt == V_INT) {
          if(ui.i < -128 || ui.i > 0xff)
            warning("Expression out of range and will be truncated");
          addbytetoarea(&odata, iv & 0xff);
        } else if(vt == V_STRING) {
          while(*sv) {
            addbytetoarea(&odata, *sv);
            sv++;
          }
        }
        tok = gettokspc(&restpart);
      }
      break;
    case DT_DCW:
      tok = gettokspc(&restpart);
      while(tok) {
        vt = evaluate(tok, &iv, &sv);
        ui.u = iv;
        if(vt == V_INT) {
          if(ui.i < -32768 || ui.i > 0xffff)
            warning("Expression out of range and will be truncated");
          addbytetoarea(&odata, iv & 0xff);
          addbytetoarea(&odata, (iv & 0xff00) >> 8);
        } else
          error("Expression does not evaluate to int");
        tok = gettokspc(&restpart);
      }
      break;
    case DT_DCD:
    case DT_DWD:
      tok = gettokspc(&restpart);
      while(tok) {
        lname = labeloffset(tok, &off);
        if(lname) {
          /* add link */
          if(*lname == '.') {
            if((vp = getvaluepos(&llabels, lname)) == NULL) {
              setbackpatch(lname, off, BP_DFNL);
              addinttoarea(&odata, 0xfddddddd);
            } else {
              if(getvaluetype(vp) != V_INT)
                error("Illegal local label");
              ll = getvalueint(vp);
              at = addinttoarea(&odata, 0xfddddddd) - curfuncstart;
              if(at & 0xff000000)
                fatal("a_declare(): illegal `at'");
              if(type == DT_DWD)
                pwarning("DWD used to declare local label; use DCD in preference");
              addinttoarea(&olinkis, at);
              addinttoarea(&olinkis, addonestrtoarea(curfuncname));
              addinttoarea(&olinkis, off+ll-curfuncstart);
              numlinkis++;
              }
          } else {
            /* linki either DFN or DWF */
            at = addinttoarea(&odata, 0xfddddddd) - curfuncstart;
            if(at & 0xff000000)
              fatal("a_declare(): illegal `at'");
            if(type == DT_DWD)
              at |= 0x01000000;
            addinttoarea(&olinkis, at);
            addinttoarea(&olinkis, addonestrtoarea(lname));
            addinttoarea(&olinkis, off);
            numlinkis++;
          }
        } else {
          vt = evaluate(tok, &iv, &sv);
          if(vt == V_INT) {
            if(type == DT_DWD)
              pwarning("DWD used to declare constant");
            addinttoarea(&odata, iv);
          } else
            error("Expression does not evaluate to int");
        }
        tok = gettokspc(&restpart);
      }
      break;
    case DT_DBB:
      tok = gettokspc(&restpart);
      if(tok) {
        if(evaluate(tok, &nn, &sv) != V_INT)
          error("Expression does not evaluate to int");
        tok = gettokspc(&restpart);
        if(tok) {
          if(evaluate(tok, &iv, &sv) != V_INT)
            error("Expression does not evaluate to int");
          ui.u = iv;
          if(ui.i < -128 || ui.i > 0xff)
            warning("Expression out of range and will be truncated");
        } else
          iv = 0;
        num = nn;
        if(num < 0)
          error("First argument of DBB must be positive");
        else {
          for( ; num > 0 ; num--) {
            addbytetoarea(&odata, iv & 0xff);
          }
        }
      } else
        error("Syntax: DBB <number>, [<pattern>]");
      break;
    case DT_DBW:
      tok = gettokspc(&restpart);
      if(tok) {
        if(evaluate(tok, &nn, &sv) != V_INT)
          error("Expression does not evaluate to int");
        tok = gettokspc(&restpart);
        if(tok) {
          if(evaluate(tok, &iv, &sv) != V_INT)
            error("Expression does not evaluate to int");
          ui.u = iv;
          if(ui.i < -32768 || ui.i > 0xffff)
            warning("Expression out of range and will be truncated");
        } else
          iv = 0;
        num = nn;
        if(num < 0)
          error("First argument of DBW must be positive");
        else {
          for( ; num > 0 ; num--) {
            addbytetoarea(&odata, iv & 0xff);
            addbytetoarea(&odata, (iv & 0xff00) >> 8);
          }
        }
      } else
        error("Syntax: DBW <number>, [<pattern>]");
      break;
    case DT_DBD:
      tok = gettokspc(&restpart);
      if(tok) {
        if(evaluate(tok, &nn, &sv) != V_INT)
          error("Expression does not evaluate to int");
        tok = gettokspc(&restpart);
        if(tok) {
          if(evaluate(tok, &iv, &sv) != V_INT)
            error("Expression does not evaluate to int");
        } else
          iv = 0;
        num = nn;
        if(num < 0)
          error("First argument of DBD must be positive");
        else {
          for( ; num > 0 ; num--) {
            addinttoarea(&odata, iv);
          }
        }
      } else
        error("Syntax: DBD <number>, [<pattern>]");
      break;
    default:
      fatal("Internal error: Unknown declare type");
  }
}

/***** B ******/

void a_branch(char *suf)
{
  unsigned int iv;
  char *sv, *lname;
  int off, vv;
  struct suffix sufb;
  struct Value *vp;

  if(!getsuffix(BRANCH, suf, &sufb))
    return;

  if(sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for branch instruction");

  if(*restpart == NULL) {
    error("Syntax: B <address>");
    return;
  }

  lname = labeloffset(restpart, &off);
  if(lname) {
    if(off > 0x00ffffff)
      error("Offset out of range");
    if(*lname == '.') {
      if((vp = getvaluepos(&llabels, lname)) == NULL) {
        setbackpatch(lname, sufb.condcode | sufb.l_flag | 0x0a000000 | off, BP_BRANCH);
        addinttoarea(&odata, 0);
      } else {
        if(getvaluetype(vp) != V_INT)
          error("Illegal local label");
        vv = getvalueint(vp)+off;
        if(vv&3)
          error("Offset must be aligned");
        else
          addinttoarea(&odata, sufb.condcode | sufb.l_flag | 0x0a000000 |
                         (((vv-odata.used)-8)>>2)&0x00ffffff);
      }
    } else {
      /* add link */
      if(off&3)
        error("Offset must be aligned");
      else {
        addinttoarea(&olinkis, addinttoarea(&odata, sufb.condcode | sufb.l_flag | 0x0afffffe)-curfuncstart);
        addinttoarea(&olinkis, addonestrtoarea(lname));
        addinttoarea(&olinkis, off);
        numlinkis++;
      }
    }
  } else {
    if(evaluate(restpart, &iv, &sv) == V_INT) {
      if(iv&3)
        error("Offset must be aligned");
      else
        addinttoarea(&odata, sufb.condcode | sufb.l_flag | 0x0a000000 |
                      (((iv-8)>>2) & 0x00ffffff) );
    } else
      error("Expression does not evaluate to int");
  }
}

/****** LDM/STM *****/

void a_mdtrans(char *suf, int ld)
{
  struct suffix sufb;
  unsigned int in = 0x08000000 | (ld<<20);
  char *tok, *dash;
  struct Value *pos;
  unsigned int rl = 0;
  int lowr = -1, rn, rn2, r;

  if(!getsuffix((ld == 1) ? MDTRANS_L : MDTRANS_S, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for LDM/STM");

  if(*restpart == NULL) {
    error("Syntax: LDM/STM<cc><options> <reg>[!], { <reg list> }[^]");
    return;
  }

  in |= sufb.condcode | sufb.multi;

  tok = gettok(&restpart);

  if(lastchar(tok) == '!') {
    in |= 1<<21;
    lastchar(tok) = 0;
  }

  pos = getvaluepos(&rns, tok);
  if(pos == NULL) {
    error("Unknown register `%s'", tok);
    return;
  }
  in |= getvalueint(pos)<<16;

  if(lastchar(restpart) == '^') {
    in |= 1<<22;
    lastchar(restpart) = 0;
  }

  restpart = skipw(restpart);
  if((*restpart != '{') || (lastchar(restpart) != '}')) {
    error("Register list must be enclosed in `{..}'");
    return;
  }

  restpart++;
  lastchar(restpart) = 0;

  while(*restpart) {
    tok = gettok(&restpart);
    if((dash = strchr(tok, '-')) != NULL) {
      *dash = 0;
      dash++;
      /* registers at tok & dash */
      pos = getvaluepos(&rns, tok);
      if(pos == NULL) {
        error("Unknown register `%s'", tok);
        return;
      }
      rn = getvalueint(pos);
      if(rn < lowr)
        informational("Registers not in numerical order");
      lowr = rn;

      pos = getvaluepos(&rns, dash);
      if(pos == NULL) {
        error("Unknown register `%s'", dash);
        return;
      }
      rn2 = getvalueint(pos);
      if(rn2 < lowr)
        informational("Registers not in numerical order");
      lowr = rn2;

      if(rn2 < rn)
        error("Bad register range");
      for(r = rn; r <= rn2; r++) {
        if(rl & (1<<r))
          informational("Registers listed twice in list");
        rl |= 1<<r;
      }
    } else {
      pos = getvaluepos(&rns, tok);
      if(pos == NULL) {
        error("Unknown register `%s'", tok);
        return;
      }
      rn = getvalueint(pos);
      if(rn < lowr)
        informational("Registers not in numerical order");
      lowr = rn;
      if(rl & (1<<rn))
        informational("Registers listed twice in list");
      rl |= 1<<rn;
    }
  }

  if((in & (1<<21)) && (in & (1<<22)) && ((rl & 1<<15)==0))
    warning("! and ^ without r15 in list");

  addinttoarea(&odata, in | rl);
}

/********* LDR/STR *********/

void a_dtrans(char *suf, int ld)
{
  unsigned int in = 0x4000000 | (ld << 20);
  struct suffix sufb;
  char *lname, *sv, *tok;
  int off, con, ra, rb, rs, doit = 1;
  struct Value *vp;
  unsigned int iv;

  if(!getsuffix(DTRANS, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.s_flag || sufb.p_flag)
    error("Illegal option for data transfer instruction");

  if(*restpart == NULL) {
    error("Syntax: LDR/STR <reg>, <address>");
    return;
  }

  ra = getreg();
  in |= ra<<12;

  in |= sufb.condcode | sufb.b_flag | sufb.t_flag;;

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
      rb = getreg();
      in |= rb<<16;
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
          if(off < -4095 || off > 4095) {
            if(temptype == -2)
              error("Offset out of range - no temporary register set");
            else if(temptype == -1 ||
              ((ld && temptype == rb) || (!ld && (temptype == ra || temptype == rb)))) {
              rs = 0;
              while(ra == rs || rb == rs) {
                rs++;
              }
              /* stmfd sp!,{rs}	*/
              addinttoarea(&odata, 0x092d0000 | sufb.condcode | (1<<rs));
              /* mov rs, #iv	*/
              assmconst(rs, iv, sufb.condcode);
              /* ldr/str ra, [rb, rs]	*/
              in |= 0x2800000;
              addinttoarea(&odata, in | rs);
              /* ldmfd sp!,{rs}	*/
              addinttoarea(&odata, 0x08bd0000 | sufb.condcode | (1<<rs));
              doit = 0;
            } else {
              assmconst(temptype, off, sufb.condcode);
              addinttoarea(&odata, in | temptype | 0x2800000);
              doit = 0;
            }
          } else {
            if(off < 0)
              off = -off;
            else
              in |= 0x800000;
          }
        }
        if(doit)
          addinttoarea(&odata, in | off);
      } else {
        /* (shifted) register offset */
        in |= 0x2000000;
        if(*restpart == '-')
          restpart++;
        else
          in |= 0x800000;
        in |= getregshift(0);
        addinttoarea(&odata, in);
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
        if(off < -4095 || off > 4095)
          error("Offset out of range");
        if(off < 0)
          off = -off;
        else
          in |= 0x800000;
        addinttoarea(&odata, in | off);
      } else {
        /* (shifted) register offset */
        in |= 0x2000000;
        if(*restpart == '-')
          restpart++;
        else
          in |= 0x800000;
        in |= getregshift(0);
        addinttoarea(&odata, in);
      }
    }
  } else {
    /* pc relative */
    in |= (1<<24) | (15<<16);
    lname = labeloffset(restpart, &off);
    if(lname) {
      if(off < -0x1ff8 || off > 0x2000)
        error("Offset out of range");
      if(*lname == '.') {
        if((vp = getvaluepos(&llabels, lname)) == NULL) {
          setbackpatch(lname, in | off, BP_DTRANS);
          addinttoarea(&odata, 0);
        } else {
          if(getvaluetype(vp) != V_INT)
            error("Illegal local label");
          con = ((getvalueint(vp)+off)-odata.used)-8;
          if(con < -0xfff || con > 0xfff)
            error("Offset out of range");
          if(con < 0)
            con = -con;
          else
            in |= 1<<23;
          addinttoarea(&odata, in | con & 0xfff);
        }
      } else {
        /* add link */
        if(off < 0)
          error("Offset within function must be positive or zero");
        else {
          addinttoarea(&olinkis, addinttoarea(&odata, in | 8)-curfuncstart);
          addinttoarea(&olinkis, addonestrtoarea(lname));
          addinttoarea(&olinkis, off);
          numlinkis++;
        }
      }
    } else {
      if(evaluate(restpart, &iv, &sv) == V_INT) {
        off = iv-8;
        if(off < -0xfff || off > 0xfff)
          error("Offset out of range");
        if(off < 0)
          off = -off;
        else
          in |= 1<<23;
        addinttoarea(&odata, in | off & 0xfff);
      } else
        error("Expression does not evaluate to int");
    }
  }
}

/***** ADR *******/

void a_adr(char *suf)
{
  unsigned int in = (1<<25);
  struct suffix sufb;
  int reg, off, con;
  char *lname;
  struct Value *vp;
  unsigned int vv;

  if(!getsuffix(DATAPRO, suf, &sufb))
    return;

  if(sufb.l_flag || sufb.b_flag || sufb.p_flag || sufb.s_flag)
    error("Illegal option for ADR instruction");

  in |= sufb.condcode;
  reg = getreg();
  in |= reg<<12;
  restpart = skipw(restpart);

  lname = labeloffset(restpart, &off);
  if(lname) {
    if(*lname == '.') {
      if((vp = getvaluepos(&llabels, lname)) == NULL) {
        setbackpatch(lname, off, BP_ADR);
        addinttoarea(&odata, in);
        addinttoarea(&odata, reg);
        addinttoarea(&odata, 0);
      } else {
        if(getvaluetype(vp) != V_INT)
          error("Illegal local label");
        con = ((getvalueint(vp)+off)-odata.used)-8;
        if(con < 0) {
          con = -con;
          in |= 2<<21;
        } else
          in |= 4<<21;
        vv = con;
        addinttoarea(&odata, in | constval(&vv) | (15<<16));
        while(vv) {
          addinttoarea(&odata, in | constval(&vv) | (reg<<16));
        }
      }
    } else {
      /* add link */
      addinttoarea(&olinkis, addinttoarea(&odata, 0x0eeeeeee | sufb.condcode)-curfuncstart);
      addinttoarea(&olinkis, addonestrtoarea(lname));
      addinttoarea(&olinkis, off);
      numlinkis++;
      addinttoarea(&odata, reg);
      addinttoarea(&odata, 0);
    }
  } else
    error("Syntax: ADR <reg>, <label>|<symbol>");
}

/******* BIN ******/

void a_bin(char *suf)
{
  unsigned int iv;
  char *sv;
  int len, ns;
  FILE *f;

  if(*suf)
    error("No suffix allowed for BIN");
  if(evaluate(restpart, &iv, &sv) != V_STRING)
    error("Syntax: BIN \"<filename>\"");
  else {
    f = fopen(sv, "rb");
    if(f == NULL) {
      error("File `%s' not found", sv);
      return;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(odata.size - odata.used <= len) {
      ns = odata.used + len + 1024;
      odata.mem = chgmem(odata.mem, ns);
      odata.size = ns;
    }
    fread(odata.mem + odata.used, 1, len, f);
    odata.used += len;
    fclose(f);
  }
}
