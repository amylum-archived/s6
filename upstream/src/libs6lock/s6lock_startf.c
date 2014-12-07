/* ISC license. */

#include <errno.h>
#include "environ.h"
#include "tai.h"
#include "skaclient.h"
#include "s6lock.h"

int s6lock_startf (s6lock_t_ref a, char const *lockdir, struct taia const *deadline, struct taia *stamp)
{
  char const *cargv[3] = { S6LOCKD_PROG, lockdir, 0 } ;
  if (!lockdir) return (errno = EINVAL, 0) ;
  return skaclient2_startf(&a->connection, cargv[0], cargv, (char const *const *)environ, S6LOCK_BANNER1, S6LOCK_BANNER1_LEN, S6LOCK_BANNER2, S6LOCK_BANNER2_LEN, SKACLIENT_OPTION_WAITPID, deadline, stamp) ;
}
