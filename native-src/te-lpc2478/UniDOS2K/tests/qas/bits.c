#include "qas.h"
#include "instr.h"
#include "ainstr.h"

#define	prec_d()	if(iclass == FP_DTRANS)		\
		          ns->precision = T_PREC_D;	\
		        else				\
		          ns->precision = PREC_D;

#define	prec_e()	if(iclass == FP_DTRANS)		\
		          ns->precision = T_PREC_E;	\
		        else				\
		          ns->precision = PREC_E;

#define	prec_s()	if(iclass == FP_DTRANS)		\
		          ns->precision = T_PREC_S;	\
		        else				\
		          ns->precision = PREC_S;


#define testcc(TC, CCC)	if((w = tstrstr(suf, TC)) != NULL) {	\
			  *w = '-'; *(w+1) = '-';		\
			  ns->condcode = CCC;			\
			} else

/* tstrstr - tests to see whether he first two chars of small are the first two or
   second two chars of big */

static char *tstrstr(const char *big, const char *small)
{
  if((*big == *small) && (*(big+1) == *(small+1)))
    return big;
  return NULL;
}

int getsuffix(int iclass, char *suf, struct suffix *ns)
{
  char *w, m1, m2, *startsearch;
  int yescondcode = 1, multiok = 1;

  if(strchr(suf, '-')) {
    error("Instruction suffixes may certainly not include '-' character");
    return 0;
  }
  ns->l_flag = 0;
  ns->condcode = AL;
  ns->b_flag = 0;
  ns->s_flag = 0;
  ns->p_flag = 0;
  ns->t_flag = 0;
  ns->multi = S_NOTHING;

  if(iclass == BRANCH)
    if(strlen(suf) == 1 || strlen(suf) == 3)
      if(*suf == 'l') {
        ns->l_flag = L_FLAG;
        *suf = '-';
        suf++;
      }

  if(iclass == MDTRANS_L || iclass == MDTRANS_S) {
    /* check for multi codes */
    if(strlen(suf) < 2) {
      error("Multiple load/stores must have an options field");
      return 0;
    }
    m1 = *(suf+strlen(suf)-2);
    m2 = *(suf+strlen(suf)-1);
    switch(m1) {
      case 'd':
        if(m2 == 'a') ns->multi = DA;
        else if(m2 == 'b') ns->multi = DB;
        else multiok = 0;
        break;
      case 'e':
        if(m2 == 'a') ns->multi = ((iclass == MDTRANS_S) ? EA_S : EA_L);
        else if(m2 == 'd') ns->multi = ((iclass == MDTRANS_S) ? ED_S : ED_L);
        else multiok = 0;
        break;
      case 'f':
        if(m2 == 'a') ns->multi = ((iclass == MDTRANS_S) ? FA_S : FA_L);
        else if(m2 == 'd') ns->multi = ((iclass == MDTRANS_S) ? FD_S : FD_L);
        else multiok = 0;
        break;
      case 'i':
        if(m2 == 'a') ns->multi = IA;
        else if(m2 == 'b') ns->multi = IB;
        else multiok = 0;
        break;
      default:
        multiok = 0;
    }
    if(!multiok) {
      error("Illegal multiple load/store options");
      return 0;
    }
    *(suf+strlen(suf)-2) = '-';
    *(suf+strlen(suf)-1) = '-';
  }

  /* check for cc */
  testcc("eq", EQ)
  testcc("ne", NE)
  testcc("cs", CS)
  testcc("cc", CC)
  testcc("mi", MI)
  testcc("pl", PL)
  testcc("vs", VS)
  testcc("vc", VC)
  testcc("hi", HI)
  testcc("ls", LS)
  testcc("ge", GE)
  testcc("lt", LT)
  testcc("gt", GT)
  testcc("le", LE)
  testcc("al", AL)
  testcc("nv", NV)
  testcc("lo", CC)
  testcc("hs", CS)
    yescondcode = 0;

  if(yescondcode) {
    /* check cc was in right place */
    if((*suf != '-') && (*(suf+1) != '-')) {
      error("Condition code is in the wrong place");
      return 0;
    }
  }

  /* if dtrans, check for t flag */
  if(iclass == DTRANS) {
    if((strlen(suf) > 0) && (lastchar(suf) == 't')) {
      ns->t_flag = T_FLAG;
      *(suf+strlen(suf)-1) = '-';
    }
  }


  if(yescondcode)
    startsearch = suf+2;
  else
    startsearch = suf;

  if((w = strchr(startsearch, 'b')) != NULL) {
    ns->b_flag = B_FLAG;
    *w = '-';
  } else if((w = strchr(startsearch, 's')) != NULL) {
    ns->s_flag = S_FLAG;
    *w = '-';
  } else if((w = strchr(startsearch, 'p')) != NULL) {
    ns->p_flag = P_FLAG;
    *w = '-';
  }

  while(*suf) {
    if(*suf != '-') {
      error("Unknown instruction suffix");
      return 0;
    } else
      suf++;
  }
  return 1;
}

int getsuffix_fp(int iclass, char *suf, struct fp_suffix *ns)
{
  char *w, *startsearch;
  int yescondcode = 1;

  ns->exception = 0;
  ns->condcode = AL;
  ns->precision = S_NOTHING;
  ns->rounding = RND_N;

  /* check for cc */
  testcc("eq", EQ)
  testcc("ne", NE)
  testcc("cs", CS)
  testcc("cc", CC)
  testcc("mi", MI)
  testcc("pl", PL)
  testcc("vs", VS)
  testcc("vc", VC)
  testcc("hi", HI)
  testcc("ls", LS)
  testcc("ge", GE)
  testcc("lt", LT)
  testcc("gt", GT)
  testcc("le", LE)
  testcc("al", AL)
  testcc("nv", NV)
  yescondcode = 0;

  if(yescondcode) {
    /* check cc was in right place */
    if((*suf != '-') && (*(suf+1) != '-')) {
      error("Condition code is in the wrong place");
      return 0;
    }
  }

  if(iclass == FP_STATUS && *suf == 'e') {
    ns->exception = E_FLAG;
    *suf = '-';
  }

  if(yescondcode)
    startsearch = suf+1;
  else
    startsearch = suf;

  if(iclass == FP_DTRANS) {
    if((w = strchr(startsearch, 'p')) != NULL) {
      ns->precision = T_PREC_P;
      *w = '-';
    }
  }

  if((w = strchr(startsearch, 's')) != NULL) {
    prec_s();
    *w = '-';
  } else if((w = strchr(startsearch, 'd')) != NULL) {
    prec_d();
    *w = '-';
  } else if((w = strchr(startsearch, 'e')) != NULL) {
    prec_e();
    *w = '-';
  }

  if((w = strchr(startsearch, 'p')) != NULL) {
    ns->rounding = RND_P;
    *w = '-';
  } else if((w = strchr(startsearch, 'm')) != NULL) {
    ns->rounding = RND_M;
    *w = '-';
  } else if((w = strchr(startsearch, 'z')) != NULL) {
    ns->rounding = RND_Z;
    *w = '-';
  }

  while(*suf) {
    if(*suf != '-') {
      error("Unknown FP instruction suffix");
      return 0;
    } else
      suf++;
  }
  return 1;
}

/* split an expression such as |func|+4 into `func' and (int) 4 */
/* if expression is value+4, returned will be NULL */
char *labeloffset(char *expr, int *retoff)
{
  char *ll, *ee, *sv;
  unsigned int iv;
  int sign = 0, b = 0;

  if(*expr == '|') {
    /* definately function and offset */
    ee = ll = expr+1;
    /* find other `|' */
    for(;;) {
      if(*ee == 0) {
        error("Unmatched `|'");
        return NULL;
      } else if(*ee == '|') {
        *ee = 0;
        ee++;
        break;
      } else
        ee++;
    }
    /* ll -> label, ee -> expr */
    while(*ee == ' ')
      ee++;

    if(*ee == '+') {
      sign = 1;
      ee++;
      if(evaluate(ee, &iv, &sv) != V_INT) {
        error("Expression does not evaluate to int");
        return NULL;
      }
    } else if(*ee == '-') {
      sign = -1;
      ee++;
      if(evaluate(ee, &iv, &sv) != V_INT) {
        error("Expression does not evaluate to int");
        return NULL;
      }
    } else if(*ee == 0) {
      iv = 0;
    } else {
      error("Syntax: <label> [ +|- <offset> ]");
      return NULL;
    }
    *retoff = iv*sign;
    return ll;
  } else if((*expr >= 'a' && *expr <= 'z') ||
            (*expr >= 'A' && *expr <= 'Z') ||
            *expr == '_' || *expr == '`' || *expr == '$' || *expr == '.') {
    /* might be */
    ll = expr;
    tempbuf[b] = expr[b]; b++;
    for(;;) {
      if((expr[b] >= 'a' && expr[b] <= 'z') ||
         (expr[b] >= 'A' && expr[b] <= 'Z') ||
         (expr[b] >= '0' && expr[b] <= '9') ||
         expr[b] == '_' || expr[b] == '`' || expr[b] == '$' || expr[b] == '.') {
        tempbuf[b] = expr[b]; b++;
      } else {
        tempbuf[b] = 0;
        ee = expr+b;
        break;
      }
    }
    if(getvaluepos(&sets, tempbuf) != NULL)
      return NULL;
    else {
      while(*ee == ' ')
        ee++;

      if(*ee == '+') {
        *ee = 0;
        sign = 1;
        ee++;
        if(evaluate(ee, &iv, &sv) != V_INT) {
          error("Expression does not evaluate to int");
          return NULL;
        }
      } else if(*ee == '-') {
        *ee = 0;
        sign = -1;
        ee++;
        if(evaluate(ee, &iv, &sv) != V_INT) {
          error("Expression does not evaluate to int");
          return NULL;
        }
      } else if(*ee == 0) {
        iv = 0;
        sign = 0;
      } else {
        error("Syntax: <label> [ +|- <offset> ]");
        return NULL;
      }
      *retoff = iv*sign;
      return tempbuf;
    }
  } else {
    /* evaluate */
    return NULL;
  }
}

int oneconst(unsigned int v)
{
  int b = 0;

  if(v == 0)
    return 1;
  while((v & (3<<b)) == 0) {
    b += 2;
  }
  if((v & (0xff<<b)) == v)
    return 1;
  else
    return 0;
}

int constval(unsigned int *pv)
{
  int b = 30, val = 0, v = *pv;

  if(v == 0)
    return 0;
  while((v & (3<<b)) == 0) {
    b -= 2;
  }

  b -= 6;
  /* get 8 bits from bit b */
  if(b > 24)
    b = 24;
  if(b < 0)
    b = 0;

  val = (v >> b) & 0xff;

  *pv = v & ~(0xff << b);

  if(b == 0)
    return val;
  return val | ((0xf - ((b>>1)-1)) << 8);
}

int getnum(void)
{
  char *tok, *sv;
  unsigned int iv;

  tok = gettok(&restpart);
  if(!tok) {
    error("Expected a number found end of line");
    return -1;
  }

  if(evaluate(tok, &iv, &sv) != V_INT) {
    error("Expression does not evaluate to a number");
    return -1;
  }

  return iv;
}

int getreg(void)
{
  char *tok;
  struct Value *pos;

  tok = gettok(&restpart);
  if(!tok) {
    error("Expected a register found end of line");
    return -1;
  }
  pos = getvaluepos(&rns, tok);
  if(pos == NULL) {
    error("Unknown register `%s'", tok);
    return -1;
  }
  return getvalueint(pos);
}

int getregfp(void)
{
  char *tok;
  struct Value *pos;

  tok = gettok(&restpart);
  if(!tok) {
    error("Expected a fp register found EOL");
    return -1;
  }
  pos = getvaluepos(&fns, tok);
  if(pos == NULL) {
    error("Unknown register `%s'", tok);
    return -1;
  }
  return getvalueint(pos);
}

int getregcp(void)
{
  char *tok;
  struct Value *pos;

  tok = gettok(&restpart);
  if(!tok) {
    error("Expected a cp register found EOL");
    return -1;
  }
  pos = getvaluepos(&cns, tok);
  if(pos == NULL) {
    error("Unknown register `%s'", tok);
    return -1;
  }
  return getvalueint(pos);
}

int getregshift(int rshift)
{
  char *sv;
  char shift[4];
  int rone, sn = 0;
  unsigned int sht = 0, iv;

  rone = getreg();

  restpart = skipw(restpart);

  shift[0] = restpart[0];
  shift[1] = restpart[1];
  shift[2] = restpart[2];
  shift[3] = 0;
  if(shift[0] && shift[1] && shift[2])
    restpart += 3;

  if(shift[0]) {
    /* with shift */
    strtolower(shift);
    if(!strcmp(shift, "rrx")) {
      if(*restpart)
        error("Unexpected token after RRX");
      return rone | 0x60;
    }
    /* conv asl to lsl */
    if(shift[0]=='a' && shift[1]=='s' && shift[2]=='l')
      shift[0] = 'l';
    /* lsl r0   lsl #0  lsl#0 */
    /* skip spaces */
    while(*restpart == ' ')
      restpart++;

    if(*restpart == '#') {
      /* const shift */
      if(!strcmp(shift, "lsl"))
        sht = 0x00;
      else if(!strcmp(shift, "lsr"))
        sht = 0x20;
      else if(!strcmp(shift, "asr"))
        sht = 0x40;
      else if(!strcmp(shift, "ror"))
        sht = 0x60;
      else
        error("Unknown shift instruction");
      if(evaluate(restpart+1, &iv, &sv) != V_INT) {
        error("Expression does not evaluate to int");
        return rone;
      }
      if(iv < 0 || iv > 31) {
        error("Shift value out of range");
        iv &= 31;
      }
      return rone | sht | (iv << 7);
    } else {
      /* register shift */
      if(!rshift)
        error("Bad shift");
      if(!strcmp(shift, "lsl"))
        sht = 0x10;
      else if(!strcmp(shift, "lsr"))
        sht = 0x30;
      else if(!strcmp(shift, "asr"))
        sht = 0x50;
      else if(!strcmp(shift, "ror"))
        sht = 0x70;
      else
        error("Unknown shift instruction");
      rone |= (sht | (getreg() << 8));
      if(*restpart)
        error("Unexpected token after register");
      return rone;
    }
  } else {
    /* simple reg */
    return rone;
  }
}

int getfpconst(char *cc)
{
  if(!strcmp(cc, "10.0"))
    return 0xf;
  if(!strcmp(cc, "0.5"))
    return 0xe;

  if(strlen(cc) != 3) {
    error("Unknown floating point constant `%s'", cc);
    return 8;
  }

  if(cc[1] != '.' || cc[2] != '0') {
    error("Illegal floating point constant `%s'", cc);
    return 8;
  }
  switch(*cc) {
    case '0': return 0x8;
    case '1': return 0x9;
    case '2': return 0xa;
    case '3': return 0xb;
    case '4': return 0xc;
    case '5': return 0xd;
    default:
      error("Illegal floating point constant `%s'", cc);
      return 8;
  }
}
