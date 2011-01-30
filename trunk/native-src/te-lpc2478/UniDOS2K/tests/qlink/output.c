#include <limits.h>
#include "qlink.h"
#include "throw.h"

#define outs(s)	for(; (*vobp = *s++) != 0; vobp++);
#define ivobp()	vobp = voutbuffer + strlen(voutbuffer);


static char voutbuffer[512];
static char *vobp;

int errors = 0;

static void vprint(char *fmt, va_list ap)
{
  for (; *fmt; fmt++) {
    if (*fmt == '%')
      switch (*++fmt) {
        case 'c': {
          *vobp++ = va_arg(ap, int);
        } break;
        case 'd': {
          int n = va_arg(ap, int);
          unsigned m;
          char buf[25], *s = buf + sizeof buf;
          *--s = 0;
          if (n == INT_MIN)
            m = (unsigned)INT_MAX + 1;
          else if (n < 0)
            m = -n;
          else
            m = n;
          do {
            *--s = m%10 + '0';
          }
          while ((m /= 10) != 0);
          if (n < 0)
            *--s = '-';
          outs(s);
        } break;
        case 'o': {
          unsigned n = va_arg(ap, unsigned);
          char buf[25], *s = buf + sizeof buf;
          *--s = 0;
          do {
            *--s = (n&7) + '0';
          }
          while ((n >>= 3) != 0);
          outs(s);
        } break;
        case 'x': {
          unsigned n = va_arg(ap, unsigned);
          char buf[25], *s = buf + sizeof buf;
          *--s = 0;
          do {
            *--s = "0123456789abcdef"[n&0xf];
          }
          while ((n >>= 4) != 0);
          outs(s);
        } break;
        case 's': {
          char *s = va_arg(ap, char *);
          if (s)
            outs(s);
        } break;
        default: {
          *vobp++ = *fmt;
        } break;
      }
    else
      *vobp++ = *fmt;
  }
  *vobp++ = '\0';
}

void vfatal(char *fmt, ...)
{
  va_list ap;

  *voutbuffer = 0;
  vobp = voutbuffer;
  va_start(ap, fmt);
  sprintf(vobp, "%s: Very fatal error: ", progname); ivobp();
  vprint(fmt, ap); ivobp();
  va_end(ap);
  fprintf(stderr, "%s\n", voutbuffer);
  exit(EXIT_FAILURE);
}

void fatal(char *fmt, ...)
{
  va_list ap;

  *voutbuffer = 0;
  vobp = voutbuffer;
  va_start(ap, fmt);
//  if(!throwback)
//    sprintf(vobp, "fatal: ");
  ivobp();

  vprint(fmt, ap); ivobp();
  va_end(ap);
  if(throwback) printf("throw_\n");
//    throw_send(ThrowbackSeriousError, 0, voutbuffer);
  else
    fprintf(stderr, "%s\n", voutbuffer);
  exit(EXIT_FAILURE);
}

void error(char *fmt, ...)
{
  va_list ap;

  *voutbuffer = 0;
  vobp = voutbuffer;
  va_start(ap, fmt);
//  if(!throwback)
    sprintf(vobp, "error: ");
  ivobp();

  vprint(fmt, ap); ivobp();
  va_end(ap);
  if(throwback)  printf("throw_\n");
//    throw_send(ThrowbackError, 0, voutbuffer);
  else
    fprintf(stderr, "%s\n", voutbuffer);
  errors++;
}

void warning(char *fmt, ...)
{
  va_list ap;

  *voutbuffer = 0;
  vobp = voutbuffer;
  va_start(ap, fmt);
  if(!throwback)  printf("throw_\n");
//    sprintf(vobp, "warning: ");
  ivobp();

  vprint(fmt, ap); ivobp();
  va_end(ap);
  if(throwback)  printf("throw_\n");
//    throw_send(ThrowbackWarning, 0, voutbuffer);
  else
    fprintf(stderr, "%s\n", voutbuffer);
}
