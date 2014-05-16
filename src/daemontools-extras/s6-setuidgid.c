/* ISC license. */

#include "strerr2.h"
#include "djbunix.h"

#define USAGE "s6-setuidgid username prog..."

int main (int argc, char const *const *argv, char const *const *envp)
{
  PROG = "s6-setuidgid" ;
  if (argc < 3) strerr_dieusage(100, USAGE) ;
  if (!prot_setuidgid(argv[1]))
    strerr_diefu2sys(111, "change identity to ", argv[1]) ;
  pathexec_run(argv[2], argv+2, envp) ;
  strerr_dieexec(111, argv[2]) ;
}
