/*
 * qas - Assembler producing QObject files
 * v2.11
 * Thu 07th May 1998
 *
 * (c) Stu Smith 1998
 *
 */

#include "qas.h"
#include "throw.h"

int errors, warnings, infos, maxerrors = 20;
char *progname;

static void exit_handler(void)
{
//  if(ythrow)
//    throw_end();
}

int main(int argc, char **argv)
{
  struct ListItem *where;
  char *fname = NULL, *outname;

  atexit(exit_handler);
  assert(sizeof(int) == 4);
  progname = argv[0];
  zfile = NULL;
  lineno = -1;
  setupmem();
  initvalues();
  evalinit();
  setup(argc, argv);
  inputInit();
/*if(verbose)
    printsrcs();*/

  outname = getmem(256);
  where = sourcefilenames;

  while(where) {
    if(where->p == NULL)
      fatal("Internal: list corrupt");
    if(src)
      fatal("Internal error: Source stack non-empty");
    clearonestrs();
    errors = 0;
    warnings = 0;
    infos = 0;
//    if(throwback)
//      throw_start(where->p);
    setSource(where->p);
    if(src) {
      freemem(fname);
      fname = getmem(strlen(src->filename)+1);
      strcpy(fname, src->filename);
      makeoutname(outname, fname);
      assemble(outputfile ? outputfile : outname);
      if(verbose) {
        fprintf(stderr, "%s: Assembly of `%s' %s with %d %s, %d %s and %d %s\n",
        progname,
        fname, ((errors==0) ? "completed" : "failed"),
        errors, ((errors==1) ? "error" : "errors"),
        warnings, ((warnings==1) ? "warning" : "warnings"),
        infos, ((infos==1)? "info" : "infos"));
      }
    }
//    if(throwback)
//      throw_end();
    where = where->next;
  }
  return EXIT_SUCCESS;
}
