/* ISC license. */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uint64.h"
#include "uint.h"
#include "bytestr.h"
#include "buffer.h"
#include "strerr2.h"
#include "tai.h"
#include "djbunix.h"
#include "s6-supervise.h"

#define USAGE "s6-svstat servicedir"

int main (int argc, char const *const *argv)
{
  s6_svstatus_t status ;
  char fmt[UINT_FMT] ;
  int isup, normallyup ;
  PROG = "s6-svstat" ;
  if (argc < 2) strerr_dieusage(100, USAGE) ;
  argv++ ; argc-- ;
  if (!s6_svstatus_read(*argv, &status))
    strerr_diefu2sys(111, "read status for ", *argv) ;

  {
    struct stat st ;
    unsigned int dirlen = str_len(*argv) ;
    char fn[dirlen + 6] ;
    byte_copy(fn, dirlen, *argv) ;
    byte_copy(fn + dirlen, 6, "/down") ;
    if (stat(fn, &st) == -1)
      if (errno != ENOENT) strerr_diefu2sys(111, "stat ", fn) ;
      else normallyup = 1 ;
    else normallyup = 0 ;
  }

  taia_now_g() ;
  if (taia_future(&status.stamp)) taia_copynow(&status.stamp) ;
  taia_sub(&status.stamp, &STAMP, &status.stamp) ;

  isup = status.pid && !status.flagfinishing ;
  if (isup)
  {
    buffer_putnoflush(buffer_1small,"up (pid ", 8) ;
    buffer_putnoflush(buffer_1small, fmt, uint_fmt(fmt, status.pid)) ;
    buffer_putnoflush(buffer_1small, ") ", 2) ;
  }
  else buffer_putnoflush(buffer_1small, "down ", 5) ;

  buffer_putnoflush(buffer_1small, fmt, uint64_fmt(fmt, status.stamp.sec.x)) ;
  buffer_putnoflush(buffer_1small," seconds", 8) ;

  if (isup && !normallyup)
    buffer_putnoflush(buffer_1small, ", normally down", 15) ;
  if (!isup && normallyup)
    buffer_putnoflush(buffer_1small, ", normally up", 13) ;
  if (isup && status.flagpaused)
    buffer_putnoflush(buffer_1small, ", paused", 8) ;
  if (!isup && (status.flagwant == 'u'))
    buffer_putnoflush(buffer_1small, ", want up", 10) ;
  if (isup && (status.flagwant == 'd'))
    buffer_putnoflush(buffer_1small, ", want down", 12) ;

  if (buffer_putflush(buffer_1small, "\n", 1) == -1)
    strerr_diefu1sys(111, "write to stdout") ;
  return 0 ;
}
