#include "qas.h"
#include "eval.h"
#include "operate.h"

struct Entry operators[] = {
  "EOS",	0,
  "||",		1,
  "&&",		2,
  "|",		3,
  "^",		4,
  "&",		5,
  "==",		6,
  "!=",		7,
  "<=",		8,
  ">=",		9,
  "<",		10,
  ">",		11,
  "<<",		12,
  ">>>",	13,
  ">>",		14,
  "+",		15,
  "-",		16,
  "*",		17,
  "/",		18,
  "%",		19,
  "!",		20,
  "~",		21,
  "-",		22,
  "+",		23,
  "(",		24,
  ")",		25,
  "",		26,
  "",		27,
  "",		28,
  "",		29,
  "a number",	30,
  "an identifier (this should never happen)",
  		31,
  "a string",	32,
  "end of expression",
  		33,
  "unknown",	34,
  NULL,		0
};

int prec[] = {
  0, 1, 2, 3, 4, 5, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 11, 11, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct Entry *symtable;
static char *lexemes;
static int lastchar, lastentry;
char *lexbuf;
char *input;
static int t;
unsigned int tokenval;
int done;
static struct StackItem *stack;
static int stackpos;
int eval_error;
char *evalstring;

void evalinit(void)
{
  lexemes = getmem(LEX_MAX);
  symtable = getmem(SYM_MAX * sizeof(struct Entry));
  lexbuf = getmem(MAX_VNAME+1);
  stack = getmem(STACK_MAX * sizeof(struct StackItem));
  clearstack();
}

/* evaluate
 *
 * Fairly ineffecient expression evaluation.
 * Takes a string and evaluates it into either int, string or error.
 * If not error, places value in either intv or strv.
 * Uses the following value sets:-
 *   sets
 *   llabels
 *
 */

/* operator precedences:
 *
 * 1	||
 * 2	&&
 * 3	|
 * 4	^
 * 5	&
 * 6	== !=
 * 7	<= >= < >
 * 8	<< >>> >>
 * 9	+ -
 * 10	* / %
 * 11	- + ! ~
 */

enum ValueType evaluate(char *string, unsigned int *intv, char **strv)
{
  while(*string == ' ') {
    string++;
  }
  evalstring = string;

  /* catch common error: &hex instead of 0xhex */
  if(*string == '&') {
    error("Hex numbers must be prefixed with `0x'");
    return V_ERROR;
  }

  eval_error = 0;
  clearstack();
  input = string;
  lastchar = -1;
  lastentry = 0;
  done = 0;
  t = egettok();
  expr(1);
  if(t != DONE) {
    error("Illegal expression");
    return V_ERROR;
  }
  if(stackpos != 1) {
    error("Illegal expression: too many/few stack entries");
  } else {
    if(eval_error)
      return V_ERROR;
    if(stack[0].tok == NUM) {
      *intv = stack[0].val;
      return V_INT;
    } else if(stack[0].tok == STRING) {
      *strv = (symtable[(stack[0].val)].name) ? (symtable[(stack[0].val)].name)
					      : "<NULL>";
      return V_STRING;
    } else {
      error("Expression evaluates to bad result");
      return V_ERROR;
    }
  }
}

void expr(int k)
{
  int et;

  if(done)
    return;
  if(k > 11)
    factor();
  else {
    expr(k+1);
    while(1) {
      if(done)
        return;
      if(prec[t] == k) {
        et = t;
        expect(t);
        expr(k+1);
        emit(et, NONE);
        continue;
      } else
        return;
    }
  }
}

void factor(void)
{
  int et;

  switch(t) {
    /* handle unary expressions */
    case MINUS:
    case PLUS:
    case UNOT:
    case UCOMP:
      et = t;
      /* must convert standard +/- to unary +/- so the stack processor
       * knows what operation to perform
       */
      if(et == MINUS)
        et = UMINUS;
      if(et == PLUS)
        et = UPLUS;
      expect(t);
      factor();
      emit(et, NONE);
      break;
    case OBRACK:
      expect(OBRACK);
      expr(1);
      expect(CBRACK);
      break;
    case NUM:
      emit(NUM, tokenval);
      expect(NUM);
      break;
    case STRING:
      emit(STRING, tokenval);
      expect(STRING);
      break;
    default:
      emit(t, NONE);
      error("Unrecognized expression");
  }
}

void expect(int tok)
{
  if(t == tok)
    t = egettok();
  else {
    emit(t, NONE);
    error("Expected %s found %s\n", operators[tok].name, operators[t].name);
  }
}

void showtok(int tt, unsigned int tval)
{
  if(tt == DONE) {
    fprintf(stderr, "token DONE\n");
  }
  else if(tt == NONE) {
    fprintf(stderr, "token NONE\n");
  }
  else if(tt == EOS) {
    fprintf(stderr, "token EOS\n");
  }
  else if(tt == NUM) {
    fprintf(stderr, "number %d\n", tval);
  }
  else if(tt == STRING) {
    if(tval <= 0)
      fprintf(stderr, "Unrecognized expression\n");
    else {
      fprintf(stderr, "string %d `%s'\n", tval, (symtable[tval].name)
						? (symtable[tval].name)
						: "<NULL>");
    }
  }
  else {
    fprintf(stderr, "operator %d %s\n", tt, operators[tt].name);
  }
}

void emit(int tt, unsigned int tval)
{
  if(tt == UNKNOWN)
    error("Unknown token");
  else if(tt == DONE)
    error("Unexpected end of expression");
  else if(tt == NONE)
    error("No token in expression");
  else if(tt == EOS)
    error("End of string in expression");
  else if(tt == NUM)
    push(tt, tval);
  else if(tt == STRING) {
    if(tval <= 0)
      error("Unrecognized expression");
    else
      push(tt, tval);
  }
  else {
    /* apply operator */
    switch(tt) {
      case 1:  l_orr();      break;
      case 2:  l_and();      break;
      case 3:  b_orr();      break;
      case 4:  b_xor();      break;
      case 5:  b_and();      break;
      case 6:  l_equ();      break;
      case 7:  l_neq();      break;
      case 8:  l_leq();      break;
      case 9:  l_geq();      break;
      case 10: l_ltt();      break;
      case 11: l_gtt();      break;
      case 12: lshift();     break;
      case 13: l_rshift();   break;
      case 14: rshift();     break;
      case 15: plus();       break;
      case 16: minus();      break;
      case 17: multiply();   break;
      case 18: divide();     break;
      case 19: modulo();     break;
      case 20: l_not();      break;
      case 21: b_not();      break;
      case 22: unaryminus(); break;
      case 23: unaryplus();  break;
      default:
        error("Unknown operator");
    }
  }
}

int olookup(char *s)
{
  int p;

  for(p = 1; operators[p].name; p++) {
    if(!strcmp(operators[p].name, s))
      return p;
  }
  return 0;
}

int lookup(char *s)
{
  int p;

  for(p = lastentry; p > 0; p--) {
    if(!strcmp(symtable[p].name, s))
      return p;
  }
  return 0;
}

int insert(char *s, int tok)
{
  int len;

  len = strlen(s);
  if(lastentry+1 >= SYM_MAX)
    fatal("Symbol table full");
  if(lastchar+len+1 >= LEX_MAX)
    fatal("Lexemes array full");
  lastentry++;
  symtable[lastentry].token = tok;
  symtable[lastentry].name = &lexemes[lastchar+1];
  lastchar += len+1;
  strcpy(symtable[lastentry].name, s);
  return lastentry;
}

void clearstack(void)
{
  stackpos = 0;
}

void push(int t, unsigned int v)
{
  if(stackpos >= STACK_MAX)
    fatal("Evaluation stack full");
  stack[stackpos].tok = t;
  stack[stackpos].val = v;
  stackpos++;
}

void pop(int *t, unsigned int *v)
{
  if(stackpos <= 0) {
    error("Evaluation stack empty");
    *t = NUM;
    *v = 0;
    done = 1;
  }
  stackpos--;
  *t = stack[stackpos].tok;
  *v = stack[stackpos].val;
}

void showstack(void)
{
  int i;

  fprintf(stderr, "Stack:\n");
  for(i = 0; i < stackpos; i++) {
    showtok(stack[i].tok, stack[i].val);
  }
}
