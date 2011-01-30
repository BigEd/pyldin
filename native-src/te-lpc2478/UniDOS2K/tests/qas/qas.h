/* qas - assembler producing QObject files
 * © Stu Smith 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/*
#define DEBUG
*/

#define lastchar(s)	(*(s+strlen(s)-1))

#define TEMPBUF_SIZE	4096
#define VERS		"2.11"
#define MAX_VNAME	128

#define BUCKETS		256

#define FUNCHEADSZ	36
#define	LINKISZ		12

enum Segment {CODE = 1, BSS = 2, DATA = 3, LIT = 4};
enum {GLOBAL = 0x100, STATIC = 0x200};

enum BPType {BP_ADR = 1, BP_BRANCH, BP_DTRANS, BP_FPDTRANS, BP_DFNL};

enum ValueType {V_ERROR = 0, V_INT, V_STRING};

struct Value {
  struct Value *next;
  char *name;
  enum ValueType type;
  union {
    int i;
    char *s;
  } v;
};

struct VSet {
  struct Value *listtab[BUCKETS];
  int notagain;
};

struct ListItem {
  char *p;
  struct ListItem *next;
};

struct Source {
  FILE *fd;
  char *filename;
  int line;
  char *buf;
  char *cp;
  char *limit;
  char *end;
  int eof;
  struct Source *next;
};

struct AreaBlk {
  char *mem;
  int used;
  int size;
};

struct fp_suffix {
  unsigned int exception;
  unsigned int condcode;
  unsigned int precision;
  unsigned int rounding;
};

struct suffix {
  unsigned int l_flag;
  unsigned int condcode;
  unsigned int b_flag;
  unsigned int s_flag;
  unsigned int p_flag;
  unsigned int t_flag;
  unsigned int multi;
};

struct export {
  struct export *next;
  char *symname;
  int line;
};

extern struct AreaBlk oheaders;
extern struct AreaBlk ostrings;
extern struct AreaBlk odata;
extern struct AreaBlk olinkis;

extern struct VSet llabels, rns, fns, cns, sets;

extern struct export *exportlist;

extern int verbose, throwback;
extern char *outputfile, *tempbuf;
extern struct ListItem *sourcefilenames;
extern struct Source *src;
extern int lineno, funcheadsoff;
extern int errors, warnings, infos;
extern int maxerrors, temptype;	/* 0-15 reg, -1 stack, -2 none */
extern int numfuncs, numlinkis, curfuncstart, infunc, supwarn, pedantic;
extern char *creator, *procos, *restpart, *zfilebuf, *zfile, *progname, *evalstring;
extern char *curfuncname;
extern int PARENTO;

extern void setupmem(void);
extern void *getmem(int);
extern void freemem(void *);
extern void vfatal(char *, ...);
extern void fatal(char *, ...);
extern void error(char *, ...);
extern void warning(char *, ...);
extern void pwarning(char *, ...);
extern void informational(char *, ...);
extern void setup(int, char **);
extern void addsourcefile(char *);
extern void printsrcs(void);
extern void inputInit(void);
extern void setSource(char *);
extern void unsetSource(void);
extern void fillbuf(void);
extern char *getline(void);
extern void assemble(char *);
extern int addtoarea(struct AreaBlk *, void *, int);
extern int addstrtoarea(struct AreaBlk *, char *);
extern int extendarea(struct AreaBlk *, int);
extern void placeint(struct AreaBlk *, int, int);
extern int addinttoarea(struct AreaBlk *, int);
extern int addbytetoarea(struct AreaBlk *, unsigned char);
extern void saveblock(char *, void *, int);
extern void makeoutname(char *, char *);
extern void makearea(struct AreaBlk *);
extern void droparea(struct AreaBlk *);
extern void alignarea(struct AreaBlk *);
extern int getint(struct AreaBlk *, int);
extern void assmline(char *);
extern void parseinit(void);
extern void initvalues(void);
extern void *chgmem(void *, int);
extern void clearvalues(struct VSet *);
extern void addintvalue(struct VSet *, char *, int);
extern void addstrvalue(struct VSet *, char *, char *);
extern struct Value *getvaluepos(struct VSet *, char *);
extern enum ValueType getvaluetype(struct Value *);
extern int getvalueint(struct Value *);
extern char *getvaluestr(struct Value *);
extern void endprevfunc(void);
extern enum ValueType evaluate(char *, unsigned int *, char **);
extern void evalinit(void);
extern char *gettok(char **);
extern char *gettokspc(char **);
extern void parseinstr(char *);
extern void strtolower(char *);
extern int getsuffix(int, char *, struct suffix *);
extern int getsuffix_fp(int, char *, struct fp_suffix *);
extern void setbackpatch(char *, int, enum BPType);
extern void backpatchlabel(char *);
extern void initbackpatch(void);
extern int backpatchfend(void);
extern char *skipw(char *);
extern int hash(char *);
extern int oneconst(unsigned int);
extern int constval(unsigned int *);
extern void clearonestrs(void);
extern int addonestrtoarea(char *);
