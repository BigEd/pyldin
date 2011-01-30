#include "qlink.h"
#include <assert.h>

#define GROW	(4*1024)

struct Block bstrings;
struct Block bdata;
struct Block bdebug;
struct Block blinkis;
struct Block bocode, bodata, bolit, boutput;

void *getmem(int size)
{
  void *mem;

  mem = malloc(size);
  if(!mem)
    fatal("Insufficient memory: Can't allocate block");
  return mem;
}

void *chgmem(void *blk, int newsize)
{
  void *mem;

  assert(blk);
  mem = realloc(blk, newsize);
  if(!mem)
    fatal("Insufficient memory: Can't reallocate block");
  return mem;
}

void freemem(void *mem)
{
  assert(mem);
  if(mem)
    free(mem);
}

void setupmem(void)
{
  bstrings.mem = NULL;
  bstrings.mem = NULL;
  bdata.mem = NULL;
  bdebug.mem = NULL;
  blinkis.mem = NULL;
  bocode.mem = NULL;
  bodata.mem = NULL;
  bolit.mem = NULL;
  boutput.mem = NULL;
}

void makeblock(struct Block *blk)
{
  blk->mem = getmem(GROW);
  blk->size = GROW;
  blk->used = 0;
}

void dropblock(struct Block *blk)
{
  assert(blk);
  freemem(blk->mem);
  blk->mem = NULL;
}

char *getstr(struct Block *blk, int off)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  assert((off >= 0) && (off < blk->used));
  return (blk->mem)+off;
}

int getint(struct Block *blk, int off)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  assert((off >= 0) && (off < blk->used));
  assert(off%sizeof(int) == 0);
  return *( (int *)((blk->mem)+off) );
}

void blkensure(struct Block *blk, int sz)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  if(blk->used + sz >= blk->size) {
    blk->mem = chgmem(blk->mem, blk->used + sz + GROW);
    blk->size = blk->used + sz + GROW;
  }
}

int addstr(struct Block *blk, char *str)
{
  int len = strlen(str)+1;

  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  blkensure(blk, len);
  strcpy(blk->mem + blk->used, str);
  blk->used += len;
  return blk->used - len;
}

int addint(struct Block *blk, int i)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  blkensure(blk, 4);
  *(int *)(blk->mem + blk->used) = i;
  blk->used += 4;
  return blk->used - 4;
}

int loaddata(struct Block *blk, FILE *f, int off, int len)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  assert(off >= 0);
  blkensure(blk, len);
  if(fseek(f, off, SEEK_SET))
    fatal("Can't load data (corrupt QObj?)");
  fread(blk->mem + blk->used, 1, len, f);
  blk->used += len;
  return blk->used - len;
}

int getdata(struct Block *blk, unsigned char *f, int off, int len)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  assert(off >= 0 && len >= 0);
  blkensure(blk, len);
  memcpy(blk->mem + blk->used, f + off, len);
  blk->used += len;
  return blk->used - len;
}

int addtoarea(struct Block *zarea, void *data, int sz)
{
  int ns;

  assert(zarea);
  if(zarea->mem == NULL)
    makeblock(zarea);
  assert(zarea->mem);
  if(zarea->used + sz >= zarea->size) {
    ns = zarea->used + sz + GROW;
    zarea->mem = chgmem(zarea->mem, ns);
    zarea->size = ns;
  }
  memcpy(zarea->mem + zarea->used, data, sz);
  zarea->used += sz;
  return zarea->used - sz;
}

void alignarea(struct Block *a)
{
  char zeros[4] = {'\0', '\0', '\0', '\0'};
  int needed;

  assert(a);
  if(a->mem == NULL)
    makeblock(a);
  assert(a->mem);
  needed = 4 - (a->used % 4);
  if(needed == 4)
    return;
  addtoarea(a, zeros, needed);
}

void bpagealign(struct Block *blk)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  addzblock(blk, 4096 - (blk->used & 0xfff));
}

int addzblock(struct Block *blk, int sz)
{
  assert(blk);
  if(blk->mem == NULL)
    makeblock(blk);
  assert(blk->mem);
  blkensure(blk, sz);
  memset(blk->mem + blk->used, 0, sz);
  blk->used += sz;
  return blk->used - sz;
}

void placeint(struct Block *zb, int zint, int off)
{
  assert(zb);
  if(zb->mem == NULL)
    makeblock(zb);
  assert(zb->mem);
  if(off%sizeof(int) != 0)
    error("Address not int-aligned", off);
  if((off > zb->used-4) || (off < 0))
    fatal("Internal error: Offset outside area");
  *(int *)(zb->mem + off) = zint;
}
