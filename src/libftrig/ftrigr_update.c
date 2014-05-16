/* ISC license. */

#include <errno.h>
#include "error.h"
#include "uint16.h"
#include "genalloc.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "ftrigr.h"

static int msghandler (void *stuff, char const *s, unsigned int n)
{
  ftrigr_ref a = (ftrigr_ref)stuff ;
  ftrigr1_t *p ;
  uint16 id ;
  if (n != 4) return (errno = EPROTO, 0) ;
  uint16_unpack_big(s, &id) ;
  p = GENSETDYN_P(ftrigr1_t, &a->data, id) ;
  if (!p) return 1 ;
  if (p->state != FR1STATE_LISTENING) return (errno = EINVAL, 0) ;
  if (!genalloc_readyplus(uint16, &a->list, 1)) return 0 ;
  switch (s[2])
  {
    case 'd' :
      p->state = FR1STATE_WAITACK ;
      break ;
    case '!' :
      if (p->options & FTRIGR_REPEAT) p->count++ ;
      else p->state = FR1STATE_WAITACKDATA ;
      break ;
    default : return (errno = EPROTO, 0) ;
  }
  p->what = s[3] ;
  id++ ; genalloc_append(uint16, &a->list, &id) ;
  return 1 ;
}

int ftrigr_update (ftrigr_ref a)
{
  genalloc_setlen(uint16, &a->list, 0) ;
  return skaclient2_update(&a->connection, &msghandler, a) ;
}
