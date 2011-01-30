#include "qas.h"
#include "throw.h"

#define BUFSIZE	4096
#define MAXLINE	512

char *line;
int lineno;
char *zfile;

struct Source *src = NULL;

void fillbuf(void)
{
  int s;

  assert(src);
  if(src->eof)
    fatal("Internal error: Can't fill buffer when EOF");
  /* move chars from cp..limit to buf */
  memmove(src->buf, src->cp, src->limit - src->cp);
  src->limit -= src->cp - src->buf;
  assert(src->limit >= src->buf);
  src->cp = src->buf;

  s = fread(src->limit, sizeof(char), src->end - src->limit, src->fd);
  if(s < src->end - src->limit)
    src->eof = 1;
  src->limit += s;
  assert(src->limit <= src->end);
  *(src->limit) = '\0';
}

char *getline(void)
{
  char c, *li = line;
  int len = 0;

  assert(src);
  lineno = src->line;
  do {
    c = *(src->cp)++;
    if(c == '\t') c = ' ';
    if(c < 32) {
      if(c != '\n')
        warning("Illegal (control) character found in source");
      c = '\0';
      src->line++;
    }
    *li++ = c;
    if(++len >= MAXLINE) {
      error("Line too long");
      unsetSource();
      *li++ = '\0';
      return line;
    }
    if(src->cp >= src->limit) {
      if(src->eof) {
        *li++ = '\0';
        unsetSource();
        return line;
      } else {
        fillbuf();
      }
    }
  } while(c != '\0');
  return line;
}

void inputInit(void)
{
  line = getmem(sizeof(char) * MAXLINE);
}

/* slight hack:
   When the last line is got, the source is unset.
   Thus errors are not reported properly. The previous line is held in lineno,
   and the last used filename is in zfile.
   zfile -> zfilebuf or NULL.
*/

void setSource(char *fn)
{
  FILE *ft;
  struct Source *t;

  if((ft = fopen(fn, "r")) == NULL) {
    error("File `%s' not found", fn);
    return;
  }
  t = getmem(sizeof(struct Source));
  t->next = src;
  src = t;
  src->fd = ft;
  src->filename = getmem(strlen(fn)+1);
  strcpy(src->filename, fn);
  strcpy(zfilebuf, fn);
  zfile = zfilebuf;
//  if(throwback && ythrow)
//    throw_file(zfile);
  src->line = 1;
  lineno = 1;
  src->buf = getmem(BUFSIZE+1 + MAXLINE+1);
  src->cp = src->buf;
  src->limit = src->buf;
  src->end = src->buf + BUFSIZE + MAXLINE;
  src->eof = 0;
  fillbuf();
}

void unsetSource(void)
{
  struct Source *n;

  if(src == NULL)
    fatal("Internal error: Can't unset Source");
  fclose(src->fd);
  freemem(src->filename);
  freemem(src->buf);
  n = src->next;
  freemem(src);
  src = n;
  if(src)
    strcpy(zfilebuf, src->filename);
//  if(throwback && ythrow)
//    throw_file(zfile);
}

/* gettokspc - takes a string and returns -> first token, -> rest of string */
/* splits at commas */

char *gettokspc(char **rest)
{
  char *line = *rest;
  char *tok;
  int quote = 0, first = 1;

  /* skip whitespace */
  for(;;) {
    if(*line == 0) {
      *rest = line;
      return NULL;
    }
    if(*line == ',' || *line == '\t' || *line == ' ') {
      line++;
      first = 0;
    } else
      break;
  }

  tok = line;

  /* skip to whitespace */
  for(;;) {
    if(*line == '"') {
      if(quote == '"'  && first == 0 && *(line-1) != '\\')
        quote = 0;
      else
        if(quote == 0)
          quote = '"';
    }
    if(*line == '\'') {
      if(quote == '\'' && first == 0 && *(line-1) != '\\')
        quote = 0;
      else
        if(quote == 0)
          quote = '\'';
    }

    if(*line == 0) {
      if(quote)
        error("Unterminated quote");
      *line = 0;
      *rest = skipw(line);
      return tok;
    }

    if(quote == 0) {
      if(*line == ',' || *line == '\t') {
        *line = 0;
        *rest = skipw(line+1);
        return tok;
      }
    }
    line++;
    first = 0;
  }
}

/* gettok - takes a string and returns -> first token, -> rest of string */
/* splits at whitespace and ',' */

char *gettok(char **rest)
{
  char *line = *rest;
  char *tok;
  int quote = 0, first = 1;

  /* skip whitespace */
  for(;;) {
    if(*line == 0) {
      *rest = line;
      return NULL;
    }
    if(*line == ' ' || *line == '\t' || *line == ',') {
      line++;
      first = 0;
    } else
      break;
  }

  tok = line;

  /* skip to whitespace */
  for(;;) {
    if(*line == '"') {
      if(quote == '"' && first == 0 && *(line-1) != '\\')
        quote = 0;
      else
        if(quote == 0)
          quote = '"';
    }
    if(*line == '\'') {
      if(quote == '\'' && first == 0 && *(line-1) != '\\')
        quote = 0;
      else
        if(quote == 0)
          quote = '\'';
    }

    if(*line == 0) {
      if(quote)
        error("Unterminated quote");
      *line = 0;
      *rest = skipw(line);
      return tok;
    }

    if(quote == 0) {
      if(*line == ' ' || *line == '\t' || *line == ',') {
        *line = 0;
        *rest = skipw(line+1);
        return tok;
      }
    }
    line++;
    first = 0;
  }
}

char *skipw(char *s)
{
  while(*s == ' ' || *s == ',') {
    s++;
  }
  return s;
}
