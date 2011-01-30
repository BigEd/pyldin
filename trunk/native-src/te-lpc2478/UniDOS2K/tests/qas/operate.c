#include "qas.h"
#include "eval.h"

static int tok;
static unsigned int val;

static int op_getint(char *opname, unsigned int *where)
{
  pop(&tok, &val);
  if(tok == NUM)
    *where = val;
  else {
    error("Illegal use of %s operator", opname);
    eval_error = 1;
    return 0;
  }
  return 1;
}

#define binary_op(f_name, op_str, op_op)	void f_name(void)		\
						{				\
						  unsigned int n1, n2;		\
						  op_getint(op_str, &n1);	\
						  op_getint(op_str, &n2);	\
						  push(NUM, n2 op_op n1);	\
						}

#define unary_op(f_name, op_str, op_op)		void f_name(void)		\
						{				\
						  unsigned int n;		\
						  op_getint(op_str, &n);	\
						  push(NUM, op_op n);		\
						}

binary_op(l_orr,	"|| (logical or)",		||	)
binary_op(l_and,	"&& (logical and)",		&&	)
binary_op(b_orr,	"| (binary or)",		|	)
binary_op(b_xor,	"^ (binary xor)",		^	)
binary_op(b_and,	"& (binary and)",		&	)
binary_op(l_equ,	"== (equality)",		==	)
binary_op(l_neq,	"!= (inequality)",		!=	)
binary_op(l_leq,	"<= (less-or-equal)",		<=	)
binary_op(l_geq,	">= (greater-or-equal)",	>=	)
binary_op(l_ltt,	"< (less than)",		<	)
binary_op(l_gtt,	"> (greater than)",		>	)
binary_op(lshift,	"<< (left shift)",		<<	)
binary_op(l_rshift,	">>> (logical right shift)",	>>	)
binary_op(plus,		"+ (addition)",			+	)
binary_op(minus,	"- (subtraction)",		-	)
binary_op(multiply,	"* (multiplication)",		*	)
binary_op(divide,	"/ (integer divide)",		/	)
binary_op(modulo,	"% (modulus)",			%	)

unary_op(l_not,		"! (logical not)",		!	)
unary_op(b_not,		"~ (complement)",		~	)
unary_op(unaryplus,	"+ (unary identity)",		+	)

void unaryminus(void)
{
  unsigned int n;

  op_getint("- (unary negation)", &n);
  push(NUM, (~n)+1);
}

void rshift(void)
{
  unsigned int n1, n2;
  int nn;

  op_getint(">> (arithmetic right shift)", &n1);
  op_getint(">> (arithmetic right shift)", &n2);
  nn = n1;
  nn = nn >> n2;
  n1 = nn;
  push(NUM, n1);
}
