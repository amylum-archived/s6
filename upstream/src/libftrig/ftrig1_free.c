/* ISC license. */

#include <unistd.h>
#include "stralloc.h"
#include "djbunix.h"
#include "ftrig1.h"

void ftrig1_free (ftrig1_ref p)
{
  if (p->name.s)
  {
    unlink(p->name.s) ;
    stralloc_free(&p->name) ;
  }
  if (p->fd >= 0)
  {
    fd_close(p->fd) ;
    p->fd = -1 ;
  }
  if (p->fdw >= 0)
  {
    fd_close(p->fdw) ;
    p->fdw = -1 ;
  }
}
