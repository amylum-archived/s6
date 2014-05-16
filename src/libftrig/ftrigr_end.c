/* ISC license. */

#include "genalloc.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "ftrigr.h"

void ftrigr_end (ftrigr_ref a)
{
  gensetdyn_free(&a->data) ;
  genalloc_free(uint16, &a->list) ;
  skaclient2_end(&a->connection) ;
  *a = ftrigr_zero ;
}
