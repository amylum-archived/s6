/* ISC license. */

#include <sys/types.h>
#include <errno.h>
#include "allreadwrite.h"
#include "fmtscan.h"
#include "buffer.h"
#include "strerr2.h"
#include "tai.h"
#include "djbtime.h"
#include "stralloc.h"
#include "skamisc.h"

int main (void)
{
  PROG = "s6-tai64nlocal" ;
  for (;;)
  {
    unsigned int p = 0 ;
    int r = skagetln(buffer_0f1, &satmp, '\n') ;
    if (r == -1)
      if (errno != EPIPE)
        strerr_diefu1sys(111, "read from stdin") ;
      else r = 1 ;
    else if (!r) break ;
    if (satmp.len > TIMESTAMP)
    {
      struct taia a ;
      p = timestamp_scan(satmp.s, &a) ;
      if (p)
      {
        char fmt[LOCALTMN_FMT+1] ;
        localtmn_t local ;
        unsigned int len ;
        localtmn_from_taia(&local, &a, 1) ;
        len = localtmn_fmt(fmt, &local) ;
        fmt[len++] = ' ' ;
        if (buffer_putalign(buffer_1, fmt, len) < 0)
          strerr_diefu1sys(111, "write to stdout") ;
      }
    }
    if (buffer_putalign(buffer_1, satmp.s + p, satmp.len - p) < 0)
      strerr_diefu1sys(111, "write to stdout") ;
    satmp.len = 0 ;
  }
  return 0 ;
}
