/* ISC license. */

#include <errno.h>
#include "uint16.h"
#include "uint32.h"
#include "bytestr.h"
#include "siovec.h"
#include "tai.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "s6lock.h"

int s6lock_acquire (s6lock_t_ref a, uint16 *u, char const *path, uint32 options, struct taia const *limit, struct taia const *deadline, struct taia *stamp)
{
  unsigned int pathlen = str_len(path) ;
  char tmp[23] = "--<" ;
  siovec_t v[2] = { { .s = tmp, .len = 23 }, { .s = path, .len = pathlen } } ;
  unsigned int i ;
  if (!gensetdyn_new(&a->data, &i)) return 0 ;
  uint16_pack_big(tmp, (uint16)i) ;
  uint32_pack_big(tmp+3, options) ;
  tain_pack(tmp+7, limit) ;
  uint32_pack_big(tmp+19, (uint32)pathlen) ;
  if (!skaclient2_sendv(&a->connection, v, 2, deadline, stamp)
   || !skaclient2_getack(&a->connection, deadline, stamp))
  {
    gensetdyn_delete(&a->data, i) ;
    return 0 ;
  }
  *GENSETDYN_P(char, &a->data, i) = EAGAIN ;
  *u = i ;
  return 1 ;
}
