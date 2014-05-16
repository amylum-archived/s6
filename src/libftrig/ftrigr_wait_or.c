/* ISC license. */

#include <errno.h>
#include "error.h"
#include "uint16.h"
#include "tai.h"
#include "iopause.h"
#include "ftrigr.h"

int ftrigr_wait_or (ftrigr_ref a, uint16 const *idlist, unsigned int n, struct taia const *deadline, struct taia *stamp, char *c)
{
  iopause_fd x = { -1, IOPAUSE_READ | IOPAUSE_EXCEPT, 0 } ;
  x.fd = ftrigr_fd(a) ;
  if (x.fd < 0) return -1 ;
  for (;;)
  {
    register unsigned int i = 0 ;
    register int r ;
    for (; i < n ; i++)
    {
      r = ftrigr_check(a, idlist[i], c) ;
      if (r < 0) return r ;
      else if (r) return i ;
    }
    r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (r < 0) return 0 ;
    else if (!r) return (errno = ETIMEDOUT, -1) ;
    else if (ftrigr_update(a) < 0) return -1 ;
  }
  return (errno = EPROTO, -1) ; /* can't happen */
}
