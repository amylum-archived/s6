/* ISC license. */

#include <errno.h>
#include "uint16.h"
#include "gensetdyn.h"
#include "ftrigr.h"

int ftrigr_check (ftrigr_ref a, uint16 id, char *c)
{
  ftrigr1_t *p ;
  if (!id--) return (errno = EINVAL, -1) ;
  p = GENSETDYN_P(ftrigr1_t, &a->data, id) ;
  if (!p) return (errno = EINVAL, -1) ;
  switch (p->state)
  {
    case FR1STATE_WAITACKDATA :
    {
      *c = p->what ;
      *p = ftrigr1_zero ;
      gensetdyn_delete(&a->data, id) ;
      return 1 ;
    }
    case FR1STATE_LISTENING :
    {
      register unsigned int r = p->count ;
      if (r) *c = p->what ;
      p->count = 0 ;
      return (int)r ;
    }
    case FR1STATE_WAITACK :
    {
      errno = p->what ;
      *p = ftrigr1_zero ;
      gensetdyn_delete(&a->data, id) ;
      return -1 ;
    }
    default: return (errno = EINVAL, -1) ;
  }
  return 0 ;
}
