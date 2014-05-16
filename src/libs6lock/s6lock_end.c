/* ISC license. */

#include "genalloc.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "s6lock.h"

void s6lock_end (s6lock_t_ref a)
{
  gensetdyn_free(&a->data) ;
  genalloc_free(uint16, &a->list) ;
  skaclient2_end(&a->connection) ;
  *a = s6lock_zero ;
}
