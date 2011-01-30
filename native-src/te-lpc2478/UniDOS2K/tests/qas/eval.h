/* eval.h */

#define STACK_MAX	100
#define	SYM_MAX		100
#define	LEX_MAX		1024
#define NONE	-1
#define EOS	0
#define MINUS	16
#define	PLUS	15
#define	UNOT	20
#define	UCOMP	21
#define	OBRACK	24
#define CBRACK	25
#define NUM	30
#define NU_ID	31
#define STRING	32
#define DONE	33
#define UNKNOWN	34
#define UMINUS	22
#define UPLUS	23

extern void expr(int);
extern void emit(int, unsigned int);
extern int lookup(char *);
extern int olookup(char *);
extern int insert(char *, int);
extern int egettok(void);
extern void test(int);
extern void factor(void);
extern void expect(int);
extern void clearstack(void);
extern void push(int, unsigned int);
extern void pop(int *, unsigned int *);
extern void showstack(void);
extern void showtok(int, unsigned int);

struct Token {
  int type;
  union {
    unsigned int i;
    char *s;
  } v;
};

struct Entry {
  char *name;
  int token;
};

struct StackItem {
  int tok;
  unsigned int val;
};

extern int eval_error;
extern unsigned int tokenval;
extern char *input;
extern int done;
extern char *lexbuf;
extern struct Entry *symtable;
