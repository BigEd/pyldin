/*
 * qlink - linker for QObject files
 * v2.05
 * Sat 07th February 1998
 *
 * (c) Stu Smith 1998
 *
 */

#include "qlink.h"
#include "throw.h"

char *progname;
FILE *openfile = NULL;

static void closeopen(void)
{
  if(openfile)
    fclose(openfile);
//  if(ythrow)
//    throw_end();
}

int main(int argc, char **argv)
{
  atexit(closeopen);
  assert(sizeof(int) == 4);
  progname = argv[0];
  setupmem();
  setup(argc, argv);
  makefnlist();
//  if(throwback)
//    throw_start("(Command line)");
  if(join) {
    initjoin();
    dojoin();
  } else
    link();
//  if(throwback)
//    throw_end();
  return EXIT_SUCCESS;
}
