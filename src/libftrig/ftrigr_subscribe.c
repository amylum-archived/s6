/* ISC license. */

#include <errno.h>
#include "uint16.h"
#include "uint32.h"
#include "siovec.h"
#include "tai.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "ftrigr.h"

uint16 ftrigr_subscribe (ftrigr_ref a, char const *path, char const *re, uint32 options, struct taia const *deadline, struct taia *stamp)
{
  unsigned int pathlen = str_len(path) ;
  unsigned int relen = str_len(re) ;
  unsigned int i ;
  register ftrigr1_t *p ;
  char tmp[15] = "--L" ;
  siovec_t v[3] = { { .s = tmp, .len = 15 }, { .s = path, .len = pathlen + 1 }, { .s = re, .len = relen } } ;
  if (!gensetdyn_new(&a->data, &i)) return 0 ;
  uint16_pack_big(tmp, (uint16)i) ;
  uint32_pack_big(tmp+3, options) ;
  uint32_pack_big(tmp+7, (uint32)pathlen) ;
  uint32_pack_big(tmp+11, (uint32)relen) ;
  if (!skaclient2_sendv(&a->connection, v, 3, deadline, stamp)
   || !skaclient2_getack(&a->connection, deadline, stamp))
  {
    gensetdyn_delete(&a->data, i) ;
    return 0 ;
  }
  p = GENSETDYN_P(ftrigr1_t, &a->data, i) ;
  p->options = options ;
  p->state = FR1STATE_LISTENING ;
  p->count = 0 ;
  p->what = 0 ;
  return (uint16)(i+1) ;
}
