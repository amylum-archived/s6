/* ISC license. */

#include <errno.h>
#include "uint16.h"
#include "tai.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "ftrigr.h"

int ftrigr_unsubscribe (ftrigr_ref a, uint16 i, struct taia const *deadline, struct taia *stamp)
{
  ftrigr1_t *p ;
  if (!i--) return (errno = EINVAL, 0) ;
  p = GENSETDYN_P(ftrigr1_t, &a->data, i) ;
  if (!p) return (errno = EINVAL, 0) ;
  switch (p->state)
  {
    case FR1STATE_WAITACK :
    case FR1STATE_WAITACKDATA :
    {
      char dummy ;
      ftrigr_check(a, i+1, &dummy) ;
      return 1 ;
    }
    default : break ;
  }
  {
    char pack[3] = "--U" ;
    uint16_pack_big(pack, i) ;
    if (!skaclient2_send(&a->connection, pack, 3, deadline, stamp)) return 0 ;
  }
  if (!skaclient2_getack(&a->connection, deadline, stamp)) return 0 ;
  *p = ftrigr1_zero ;
  return gensetdyn_delete(&a->data, i) ;
}
