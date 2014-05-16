/* ISC license. */

#include "strerr2.h"
#include "s6-supervise.h"

#define USAGE "s6-svcanctl [ -phratszbnNiq0678 ] svscandir"

int main (int argc, char const *const *argv)
{
  PROG = "s6-svcanctl" ;
  return s6_svc_main(argc, argv, "phratszbnNiq0678", USAGE, ".s6-svscan") ;
}
