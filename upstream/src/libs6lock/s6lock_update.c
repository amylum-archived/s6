/* ISC license. */

#include <errno.h>
#include "error.h"
#include "uint16.h"
#include "genalloc.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "s6lock.h"

static int msghandler (void *stuff, char const *s, unsigned int n)
{
  s6lock_t_ref a = (s6lock_t_ref)stuff ;
  char *p ;
  uint16 id ;
  if (n != 3) return (errno = EPROTO, 0) ;
  uint16_unpack_big(s, &id) ;
  p = GENSETDYN_P(char, &a->data, id) ;
  if (*p == EBUSY) *p = s[2] ;
  else if (error_isagain(*p)) *p = s[2] ? s[2] : EBUSY ;
  else return (errno = EPROTO, 0) ;
  if (!genalloc_append(uint16, &a->list, &id)) return 0 ;
  return 1 ;
}

int s6lock_update (s6lock_t_ref a)
{
  genalloc_setlen(uint16, &a->list, 0) ;
  return skaclient2_update(&a->connection, &msghandler, a) ;
}
