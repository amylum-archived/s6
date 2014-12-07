/* ISC license. */

#include "tai.h"
#include "skaclient.h"
#include "ftrigr.h"

int ftrigr_start (ftrigr_ref a, char const *s, struct taia const *deadline, struct taia *stamp)
{
  return skaclient2_start(&a->connection, s, FTRIGR_BANNER1, FTRIGR_BANNER1_LEN, FTRIGR_BANNER2, FTRIGR_BANNER2_LEN, deadline, stamp) ;
}
