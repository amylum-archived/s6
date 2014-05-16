/* ISC license. */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "bytestr.h"
#include "strerr2.h"
#include "djbunix.h"
#include "s6-supervise.h"

int s6_supervise_lock_mode (char const *subdir, unsigned int subdirmode, unsigned int controlmode)
{
  unsigned int subdirlen = str_len(subdir) ;
  int fdctl, fdctlw, fdlock ;
  char control[subdirlen + 9] ;
  char lock[subdirlen + 6] ;
  byte_copy(control, subdirlen, subdir) ;
  byte_copy(control + subdirlen, 9, "/control") ;
  byte_copy(lock, subdirlen, subdir) ;
  byte_copy(lock + subdirlen, 6, "/lock") ;
  if ((mkdir(subdir, (mode_t)subdirmode) == -1) && (errno != EEXIST))
    strerr_diefu2sys(111, "mkdir ", subdir) ;
  if (fifo_make(control, controlmode) < 0)
  {
    struct stat st ;
    if (errno != EEXIST)
      strerr_diefu2sys(111, "fifo_make ", control) ;
    if (stat(control, &st) < 0)
      strerr_diefu2sys(111, "stat ", control) ;
    if (!S_ISFIFO(st.st_mode))
      strerr_diefu2x(100, control, " is not a FIFO") ;
  }
  fdlock = open_create(lock) ;
  if (fdlock < 0)
    strerr_diefu2sys(111, "open_create ", lock) ;
  if (lock_ex(fdlock) < 0)
    strerr_diefu2sys(111, "lock ", lock) ;
  fdctlw = open_write(control) ;
  if (fdctlw >= 0) strerr_dief1x(100, "directory already locked") ;
  if (errno != ENXIO)
    strerr_diefu2sys(111, "open_write ", control) ;
  fdctl = open_read(control) ;
  if (fdctl < 0)
    strerr_diefu2sys(111, "open_read ", control) ;
  fd_close(fdlock) ;
  fdctlw = open_write(control) ;
  if (fdctlw < 0)
    strerr_diefu2sys(111, "open_write ", control) ;
  if ((coe(fdctlw) < 0) || (coe(fdctl) < 0))
    strerr_diefu2sys(111, "coe ", control) ;

  return fdctl ;
  /* fdctlw is leaking. That's okay, it's coe. */
}
