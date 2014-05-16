/* ISC license. */

#include <errno.h>
#include "error.h"
#include "uint16.h"
#include "tai.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "s6lock.h"

int s6lock_release (s6lock_t_ref a, uint16 i, struct taia const *deadline, struct taia *stamp)
{
  char *p = GENSETDYN_P(char, &a->data, i) ;
  if ((*p != EBUSY) && !error_isagain(*p))
  {
    s6lock_check(a, i) ;
    return 1 ;
  }
  {
    char pack[3] = "-->" ;
    uint16_pack_big(pack, i) ;
    if (!skaclient2_send(&a->connection, pack, 3, deadline, stamp)) return 0 ;
  }
  if (!skaclient2_getack(&a->connection, deadline, stamp)) return 0 ;
  *p = EINVAL ;
  return gensetdyn_delete(&a->data, i) ;
}
