/* ISC license. */

#include "tai.h"
#include "skaclient.h"
#include "ftrigr.h"

int ftrigr_startf (ftrigr_ref a, struct taia const *deadline, struct taia *stamp)
{
  char const *cargv[2] = { FTRIGRD_PROG, 0 } ;
  char const *cenvp[1] = { 0 } ;
  return skaclient2_startf(&a->connection, cargv[0], cargv, cenvp, FTRIGR_BANNER1, FTRIGR_BANNER1_LEN, FTRIGR_BANNER2, FTRIGR_BANNER2_LEN, SKACLIENT_OPTION_WAITPID, deadline, stamp) ;
}
