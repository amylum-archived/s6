/* ISC license. */

#include <errno.h>
#include "uint16.h"
#include "tai.h"
#include "iopause.h"
#include "ftrigr.h"

int ftrigr_wait_and (ftrigr_ref a, uint16 const *idlist, unsigned int n, struct taia const *deadline, struct taia *stamp)
{
  iopause_fd x = { -1, IOPAUSE_READ, 0 } ;
  x.fd = ftrigr_fd(a) ;
  for (; n ; n--, idlist++)
  {
    for (;;)
    {
      char dummy ;
      register int r = ftrigr_check(a, *idlist, &dummy) ;
      if (r < 0) return r ;
      else if (r) break ;
      r = iopause_stamp(&x, 1, deadline, stamp) ;
      if (r < 0) return r ;
      else if (!r) return (errno = ETIMEDOUT, -1) ;
      else if (ftrigr_update(a) < 0) return -1 ;
    }
  }
  return 1 ;
}
