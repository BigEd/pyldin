#include "qas.h"

char *tempbuf, *zfilebuf;
struct AreaBlk oheaders;
struct AreaBlk ostrings;
struct AreaBlk odata;
struct AreaBlk olinkis;

struct VSet llabels, rns, fns, cns, sets;

struct OneString {
  struct OneString *next;
  int off;	/* in ostrings */
};

static struct OneString *onestr[BUCKETS];

struct ListItem *sourcefilenames = NULL;
int numFiles;

static int scatter[] = {
  2078917053, 143302914,  1027100827, 1953210302, 755253631,  2002600785,
  1405390230, 45248011,   1099951567, 433832350,  2018585307, 438263339,
  813528929,  1703199216, 618906479,  573714703,  766270699,  275680090,
  1510320440, 1583583926, 1723401032, 1965443329, 1098183682, 1636505764,
  980071615,  1011597961, 643279273,  1315461275, 157584038,  1069844923,
  471560540,  89017443,   1213147837, 1498661368, 2042227746, 1968401469,
  1353778505, 1300134328, 2013649480, 306246424,  1733966678, 1884751139,
  744509763,  400011959,  1440466707, 1363416242, 973726663,  59253759,
  1639096332, 336563455,  1642837685, 1215013716, 154523136,  593537720,
  704035832,  1134594751, 1605135681, 1347315106, 302572379,  1762719719,
  269676381,  774132919,  1851737163, 1482824219, 125310639,  1746481261,
  1303742040, 1479089144, 899131941,  1169907872, 1785335569, 485614972,
  907175364,  382361684,  885626931,  200158423,  1745777927, 1859353594,
  259412182,  1237390611, 48433401,   1902249868, 304920680,  202956538,
  348303940,  1008956512, 1337551289, 1953439621, 208787970,  1640123668,
  1568675693, 478464352,  266772940,  1272929208, 1961288571, 392083579,
  871926821,  1117546963, 1871172724, 1771058762, 139971187,  1509024645,
  109190086,  1047146551, 1891386329, 994817018,  1247304975, 1489680608,
  706686964,  1506717157, 579587572,  755120366,  1261483377, 884508252,
  958076904,  1609787317, 1893464764, 148144545,  1415743291, 2102252735,
  1788268214, 836935336,  433233439,  2055041154, 2109864544, 247038362,
  299641085,  834307717,  1364585325, 23330161,   457882831,  1504556512,
  1532354806, 567072918,  404219416,  1276257488, 1561889936, 1651524391,
  618454448,  121093252,  1010757900, 1198042020, 876213618,  124757630,
  2082550272, 1834290522, 1734544947, 1828531389, 1982435068, 1002804590,
  1783300476, 1623219634, 1839739926, 69050267,   1530777140, 1802120822,
  316088629,  1830418225, 488944891,  1680673954, 1853748387, 946827723,
  1037746818, 1238619545, 1513900641, 1441966234, 367393385,  928306929,
  946006977,  985847834,  1049400181, 1956764878, 36406206,   1925613800,
  2081522508, 2118956479, 1612420674, 1668583807, 1800004220, 1447372094,
  523904750,  1435821048, 923108080,  216161028,  1504871315, 306401572,
  2018281851, 1820959944, 2136819798, 359743094,  1354150250, 1843084537,
  1306570817, 244413420,  934220434,  672987810,  1686379655, 1301613820,
  1601294739, 484902984,  139978006,  503211273,  294184214,  176384212,
  281341425,  228223074,  147857043,  1893762099, 1896806882, 1947861263,
  1193650546, 273227984,  1236198663, 2116758626, 489389012,  593586330,
  275676551,  360187215,  267062626,  265012701,  719930310,  1621212876,
  2108097238, 2026501127, 1865626297, 894834024,  552005290,  1404522304,
  48964196,   5816381,    1889425288, 188942202,  509027654,  36125855,
  365326415,  790369079,  264348929,  513183458,  536647531,  13672163,
  313561074,  1730298077, 286900147,  1549759737, 1699573055, 776289160,
  2143346068, 1975249606, 1136476375, 262925046,  92778659,   1856406685,
  1884137923, 53392249,   1735424165, 1602280572
};

int hash(char *str)
{
  unsigned int h = 0;
  int i = strlen(str);
  char *end = str;

  for(; i > 0; i--) {
    h = (h<<1) + scatter[*(unsigned char *)end++];
  }
  return h & (BUCKETS-1);
}

void *getmem(int size)
{
  void *mem;

  mem = malloc(size);
  if(!mem)
    fatal("Insufficient memory: Can't allocate block");
  return mem;
}

void freemem(void *mem)
{
  if(mem)
    free(mem);
}

void *chgmem(void *blk, int newsize)
{
  void *mem;

  mem = realloc(blk, newsize);
  if(!mem)
    fatal("Insufficient memory: Can't reallocate block");
  return mem;
}

void setupmem(void)
{
  tempbuf = getmem(TEMPBUF_SIZE);
  zfilebuf = getmem(256);
  oheaders.mem = NULL;
  ostrings.mem = NULL;
  odata.mem = NULL;
  olinkis.mem = NULL;
  creator = getmem(256);
  procos = getmem(256);
}

void makearea(struct AreaBlk *a)
{
  if(a->mem)
    droparea(a);
  a->mem = getmem(1024);
  a->used = 0;
  a->size = 1024;
}

void droparea(struct AreaBlk *a)
{
  freemem(a->mem);
  a->mem = NULL;
}

int addtoarea(struct AreaBlk *zarea, void *data, int sz)
{
  int ns;

  if(zarea->size - zarea->used <= sz) {
    ns = zarea->used + sz + 1024;
    zarea->mem = chgmem(zarea->mem, ns);
    zarea->size = ns;
  }
  memcpy(zarea->mem + zarea->used, data, sz);
  zarea->used += sz;
  return zarea->used-sz;
}

int addbytetoarea(struct AreaBlk *zarea, unsigned char zchar)
{
  int ns;

  if(zarea->size - zarea->used <= 1) {
    ns = zarea->used + 1 + 1024;
    zarea->mem = chgmem(zarea->mem, ns);
    zarea->size = ns;
  }
  *(zarea->mem + zarea->used) = zchar;
  zarea->used += 1;
  return zarea->used - 1;
}

int addstrtoarea(struct AreaBlk *zarea, char *str)
{
  int ns;

  if(zarea->size - zarea->used <= strlen(str)+1) {
    ns = zarea->used + strlen(str) + 1024;
    zarea->mem = chgmem(zarea->mem, ns);
    zarea->size = ns;
  }
  strcpy(zarea->mem + zarea->used, str);
  zarea->used += strlen(str)+1;
  return zarea->used-(strlen(str)+1);
}

int extendarea(struct AreaBlk *zarea, int sz)
{
  int ns;

  if(sz < 0)
    fatal("Internal: extendarea(%p, %d)", zarea, sz);
  if(zarea->size - zarea->used <= sz) {
    ns = zarea->used + sz + 1024;
    zarea->mem = chgmem(zarea->mem, ns);
    zarea->size = ns;
  }
  zarea->used += sz;
  return zarea->used-sz;
}

void placeint(struct AreaBlk *zarea, int zint, int off)
{
  if(off%sizeof(int) != 0)
    error("Address not int-aligned", off);
  if(zarea->used-4 < off || off < 0)
    fatal("Internal error: Offset outside area");
  *(int *)(zarea->mem + off) = zint;
}

int getint(struct AreaBlk *zarea, int off)
{
  if(off%sizeof(int) != 0)
    error("Address not int-aligned", off);
  if(zarea->used-4 < off || off < 0)
    fatal("Internal error: Offset outside area");
  return *(int *)(zarea->mem + off);
}

int addinttoarea(struct AreaBlk *zarea, int zint)
{
  placeint(zarea, zint, extendarea(zarea, 4));
  return zarea->used - 4;
}

void alignarea(struct AreaBlk *a)
{
  char zeros[4] = {'\0', '\0', '\0', '\0'};
  int needed;

  needed = 4 - (a->used % 4);
  if(needed == 4)
    return;
  addtoarea(a, zeros, needed);
}

void strtolower(char *str)
{
  while(*str) {
    if(*str >= 'A' && *str <= 'Z')
      *str = (*str) + ('a'-'A');
      str++;
  }
}

void initvalues(void)
{
  int i;

  for(i = 0; i < BUCKETS; i++) {
    llabels.listtab[i] = NULL;
    rns.listtab[i] = NULL;
    fns.listtab[i] = NULL;
    cns.listtab[i] = NULL;
    sets.listtab[i] = NULL;
    onestr[i] = NULL;
  }

  llabels.notagain = 1;
  rns.notagain = 0;
  fns.notagain = 0;
  cns.notagain = 0;
  sets.notagain = 0;
}

void clearvalues(struct VSet *set)
{
  int i;
  struct Value *v, *nv;

  for(i = 0; i < BUCKETS; i++) {
    v = set->listtab[i];
    while(v) {
      nv = v->next;
      freemem(v);
      v = nv;
    }
    set->listtab[i] = NULL;
  }
}

void addintvalue(struct VSet *set, char *n, int val)
{
  struct Value *p;
  int h;

  if((p = getvaluepos(set, n)) != NULL) {
    if(getvaluetype(p) != V_INT) {
      error("Value `%s' is already in use as a non-integer", n);
      return;
    }
    if(getvalueint(p) != val) {
      error("Value `%s' has already been defined with value %d", n, getvalueint(p));
      return;
    }
    if(set->notagain) {
      error("Value `%s' may not be redefined", n);
      return;
    }
    return;
  }

  h = hash(n);
  p = getmem(sizeof(struct Value));
  p->name = getmem(strlen(n)+1);
  strcpy(p->name, n);
  p->type = V_INT;
  p->v.i = val;
  p->next = set->listtab[h];
  set->listtab[h] = p;
}

void addstrvalue(struct VSet *set, char *n, char *val)
{
  struct Value *p;
  int h;

  if(strlen(n) > MAX_VNAME)
    pwarning("Value name very long");
  if((p = getvaluepos(set, n)) != NULL) {
    if(getvaluetype(p) != V_STRING) {
      error("Value `%s' is already in use as a non-string", n);
      return;
    }
    if(strcmp(getvaluestr(p), val)) {
      error("Value `%s' has already been defined with value `%s'", n, getvaluestr(p));
      return;
    }
    if(set->notagain) {
      error("Value `%s' may not be redefined", n);
      return;
    }
    return;
  }

  h = hash(n);
  p = getmem(sizeof(struct Value));
  p->name = getmem(strlen(n)+1);
  strcpy(p->name, n);
  p->type = V_STRING;
  p->v.s = getmem(strlen(val)+1);
  strcpy(p->v.s, val);;
  p->next = set->listtab[h];
  set->listtab[h] = p;
}

struct Value *getvaluepos(struct VSet *set, char *n)
{
  int h = hash(n);
  struct Value *v;

  v = set->listtab[h];

  while(v) {
    if(!strcmp(v->name, n))
      return v;
    v = v->next;
  }
  return NULL;
}

enum ValueType getvaluetype(struct Value *p)
{
  return p->type;
}

int getvalueint(struct Value *p)
{
  if(p->type != V_INT)
    fatal("Internal error: Value `%s' is not an int", p->name);
  return p->v.i;
}

char *getvaluestr(struct Value *p)
{
  if(p->type != V_STRING)
    fatal("Internal error: Value `%s' is not an string", p->name);
  return p->v.s;
}

void addsourcefile(char *filename)
{
  struct ListItem *li, *where = sourcefilenames;

  if(where == NULL) {
    where = getmem(sizeof(struct ListItem));
    where->next = NULL;
    where->p = getmem(strlen(filename)+1);
    strcpy(where->p, filename);
    sourcefilenames = where;
    return;
  }

  while(where->next) {
    where = where->next;
  }

  li = getmem(sizeof(struct ListItem));
  li->next = NULL;
  li->p = getmem(strlen(filename)+1);
  strcpy(li->p, filename);
  where->next = li;
}

void printsrcs(void)
{
  struct ListItem *where = sourcefilenames;

  fprintf(stderr, "%s: Source files:-\n", progname);
  if(where)
    do {
      if(where->p)
        fprintf(stderr, "    %s\n", where->p);
      where = where->next;
    } while(where);
  if(outputfile)
    fprintf(stderr, "%s: output file: %s\n", progname, outputfile);
}

/* add one string only to a ostrings */
int addonestrtoarea(char *str)
{
  int h = hash(str);
  struct OneString *os, *es;

  os = onestr[h];
  while(os) {
    if(!strcmp(str, ostrings.mem + os->off)) {
      return os->off;
    }
    os = os->next;
  }

  es = getmem(sizeof(struct OneString));
  es->off = addstrtoarea(&ostrings, str);
  es->next = onestr[h];
  onestr[h] = es;
  return es->off;
}

void clearonestrs(void)
{
  int i;
  struct OneString *os, *nos;

  for(i = 0; i < BUCKETS; i++) {
    os = onestr[i];
    while(os) {
      nos = os->next;
      freemem(os);
      os = nos;
    }
    onestr[i] = NULL;
  }
}
