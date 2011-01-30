/* qlink - linker for QObject files
 * © Stu Smith 1998
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QOBJ_ID	0x6a624f51

#define VERS		"2.07"
#define FBUCKETS	1024
#define FHEADSZ		36
#define HEADERSZ	60
#define LINKISZ		12

enum Segment {UNUSED = 0, CODE = 1, BSS = 2, DATA = 3, LIT = 4};
enum {GLOBAL = 0x100, STATIC = 0x200};

struct Block {
  char *mem;
  int used;
  int size;
};

struct Function {
  struct Function *next;
  int name;		/* strings */
  struct ObjFile *objfile;
  int funcnum;
  enum Segment seg;	/* or 0 for no seg */
  int scope;
  int parento;
  char *fstrings;
  int offset;
  int linked;
};

struct ObjFile {
  struct ObjFile *next;
  int filename;		/* strings */
  unsigned char *data;
};

extern char *progname;
extern int verbose, multidef, map, join, sheaders;
extern int codenames, datanames, errors, pagealign, startaddr;
extern int mapall, fussy;
extern char *outputfile, *firstfunction, *symfile, *creator, *procos;

extern FILE *openfile;

extern struct ObjFile *filelist;
extern struct Function *functab[FBUCKETS];
extern struct Block bstrings;
extern struct Block bdata;
extern struct Block bdebug;
extern struct Block blinkis;
extern struct Block bocode, bodata, bolit;
extern struct Block boutput;

extern void *getmem(int);
extern void freemem(void *);
extern void *chgmem(void *, int);
extern void setupmem(void);
extern void vfatal(char *, ...);
extern void fatal(char *, ...);
extern void error(char *, ...);
extern void warning(char *, ...);
extern char *riscos_convert(char *);
extern void setup(int, char **);
extern void saveblock(char *, void *, int);
extern void addobjectfile(char *);
extern void addobjectlibfile(char *);
extern void makeblock(struct Block *);
extern void dropblock(struct Block *);
extern char *getstr(struct Block *, int);
extern int getint(struct Block *, int);
extern int addstr(struct Block *, char *);
extern int loaddata(struct Block *, FILE *, int /* off */, int /* len */);
extern int getdata(struct Block *, unsigned char *, int /* off */, int /* len */);
extern int hash(char *, int);
extern void makefnlist(void);
extern void link(void);
extern int addtoarea(struct Block *, void *, int);
extern void alignarea(struct Block *);
extern void bpagealign(struct Block *);
extern int addzblock(struct Block *, int);
extern void placeint(struct Block *, int, int);
extern int addint(struct Block *, int);
extern void dojoin(void);
extern void initjoin(void);
extern void settfile(char *);
