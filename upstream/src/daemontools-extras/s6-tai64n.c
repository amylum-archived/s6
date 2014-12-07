/* ISC license. */

#include <errno.h>
#include "buffer.h"
#include "strerr2.h"
#include "tai.h"
#include "stralloc.h"
#include "skamisc.h"

int main (void)
{
  char stamp[TIMESTAMP+1] ;
  PROG = "s6-tai64n" ;
  stamp[TIMESTAMP] = ' ' ;
  for (;;)
  {
    register int r = skagetln(buffer_0f1, &satmp, '\n') ;
    if (r < 0)
      if (errno != EPIPE)
        strerr_diefu1sys(111, "read from stdin") ;
      else
      {
        r = 1 ;
        if (!stralloc_catb(&satmp, "\n", 1))
          strerr_diefu1sys(111, "add newline") ;
      }
    else if (!r) break ;
    timestamp(stamp) ;
    if ((buffer_putalign(buffer_1, stamp, TIMESTAMP+1) < 0)
     || (buffer_putalign(buffer_1, satmp.s, satmp.len) < 0))
      strerr_diefu1sys(111, "write to stdout") ;
    satmp.len = 0 ;
  }
  return 0 ;
}
