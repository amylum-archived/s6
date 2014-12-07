/* ISC license. */

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "uint16.h"
#include "uint32.h"
#include "allreadwrite.h"
#include "error.h"
#include "strerr2.h"
#include "buffer.h"
#include "stralloc.h"
#include "genalloc.h"
#include "bufalloc.h"
#include "sig.h"
#include "selfpipe.h"
#include "tai.h"
#include "djbunix.h"
#include "iopause.h"
#include "netstring.h"
#include "skaclient.h"
#include "s6lock.h"

#define USAGE "s6lockd lockdir"
#define X() strerr_dief1x(101, "internal inconsistency, please submit a bug-report.")

static struct taia const taia_zero = TAIA_ZERO ;

static bufalloc asyncout = BUFALLOC_ZERO ;

typedef struct s6lockio_s s6lockio_t, *s6lockio_t_ref ;
struct s6lockio_s
{
  unsigned int xindex ;
  unsigned int pid ;
  struct taia limit ;
  int p[2] ;
  uint16 id ; /* given by client */
} ;
#define S6LOCKIO_ZERO { 0, 0, TAIA_ZERO, { -1, -1 }, 0 }
static s6lockio_t const szero = S6LOCKIO_ZERO ;

static genalloc a = GENALLOC_ZERO ; /* array of s6lockio_t */

static void s6lockio_free (s6lockio_t_ref p)
{
  register int e = errno ;
  fd_close(p->p[1]) ;
  fd_close(p->p[0]) ;
  kill(p->pid, SIGTERM) ;
  *p = szero ;
  errno = e ;
}

static void cleanup (void)
{
  register unsigned int i = genalloc_len(s6lockio_t, &a) ;
  for (; i ; i--) s6lockio_free(genalloc_s(s6lockio_t, &a) + i - 1) ;
  genalloc_setlen(s6lockio_t, &a, 0) ;
}
 
static void trig (uint16 id, char e)
{
  char pack[6] = "3:ide," ;
  uint16_pack_big(pack+2, id) ;
  pack[4] = e ;
  if (!bufalloc_put(&asyncout, pack, 6))
  {
    cleanup() ;
    strerr_diefu1sys(111, "build answer") ;
  }
}

static void answer (char c)
{
  if (!bufalloc_put(bufalloc_1, &c, 1))
  {
    cleanup() ;
    strerr_diefu1sys(111, "bufalloc_put") ;
  }
}

static void remove (unsigned int i)
{
  register unsigned int n = genalloc_len(s6lockio_t, &a) - 1 ;
  s6lockio_free(genalloc_s(s6lockio_t, &a) + i) ;
  genalloc_s(s6lockio_t, &a)[i] = genalloc_s(s6lockio_t, &a)[n] ;
  genalloc_setlen(s6lockio_t, &a, n) ;
}

static void handle_signals (void)
{
  for (;;)
  {
    switch (selfpipe_read())
    {
      case -1 : cleanup() ; strerr_diefu1sys(111, "selfpipe_read") ;
      case 0 : return ;
      case SIGTERM :
      case SIGQUIT :
      case SIGHUP :
      case SIGABRT :
      case SIGINT : cleanup() ; _exit(0) ;
      case SIGCHLD : wait_reap() ; break ;
      default : cleanup() ; X() ;
    }
  }
}

int main (int argc, char const *const *argv)
{
  stralloc indata = STRALLOC_ZERO ;
  struct taia deadline ;
  unsigned int instate = 0 ;
  int sfd ;
  PROG = "s6lockd" ;
  
  if (argc < 2) strerr_dieusage(100, USAGE) ;
  if (chdir(argv[1]) < 0) strerr_diefu2sys(111, "chdir to ", argv[1]) ;
  if (ndelay_on(0) < 0) strerr_diefu2sys(111, "ndelay_on ", "0") ;
  if (ndelay_on(1) < 0) strerr_diefu2sys(111, "ndelay_on ", "1") ;
  if (sig_ignore(SIGPIPE) < 0) strerr_diefu1sys(111, "ignore SIGPIPE") ;

  sfd = selfpipe_init() ;
  if (sfd < 0) strerr_diefu1sys(111, "selfpipe_init") ;
  {
    sigset_t set ;
    sigemptyset(&set) ;
    sigaddset(&set, SIGCHLD) ;
    sigaddset(&set, SIGTERM) ;
    sigaddset(&set, SIGQUIT) ;
    sigaddset(&set, SIGHUP) ;
    sigaddset(&set, SIGABRT) ;
    sigaddset(&set, SIGINT) ;
    if (selfpipe_trapset(&set) < 0)
      strerr_diefu1sys(111, "trap signals") ;
  }
  
  taia_now_g() ;
  taia_addsec_g(&deadline, 2) ;

  if (!skaserver2_sync_g(&asyncout, S6LOCK_BANNER1, S6LOCK_BANNER1_LEN, S6LOCK_BANNER2, S6LOCK_BANNER2_LEN, &deadline))
    strerr_diefu1sys(111, "sync with client") ;

  for (;;)
  {
    register unsigned int n = genalloc_len(s6lockio_t, &a) ;
    iopause_fd x[4 + n] ;
    unsigned int i = 0 ;
    int r ;

    taia_add_g(&deadline, &infinitetto) ;
    x[0].fd = 0 ; x[0].events = IOPAUSE_EXCEPT | IOPAUSE_READ ;
    x[1].fd = 1 ; x[1].events = IOPAUSE_EXCEPT | (bufalloc_len(bufalloc_1) ? IOPAUSE_WRITE : 0) ;
    x[2].fd = bufalloc_fd(&asyncout) ; x[2].events = IOPAUSE_EXCEPT | (bufalloc_len(&asyncout) ? IOPAUSE_WRITE : 0) ;
    x[3].fd = sfd ; x[3].events = IOPAUSE_READ ;
    for (; i < n ; i++)
    {
      register s6lockio_t_ref p = genalloc_s(s6lockio_t, &a) + i ;
      x[4+i].fd = p->p[0] ;
      x[4+i].events = IOPAUSE_READ ;
      if (p->limit.sec.x && taia_less(&p->limit, &deadline)) deadline = p->limit ;
      p->xindex = 4+i ;
    }

    r = iopause_g(x, 4 + n, &deadline) ;
    if (r < 0)
    {
      cleanup() ;
      strerr_diefu1sys(111, "iopause") ;
    }

   /* timeout => seek and destroy */
    if (!r)
    {
      for (i = 0 ; i < n ; i++)
      {
        register s6lockio_t_ref p = genalloc_s(s6lockio_t, &a) + i ;
        if (p->limit.sec.x && !taia_future(&p->limit)) break ;
      }
      if (i < n)
      {
        trig(genalloc_s(s6lockio_t, &a)[i].id, ETIMEDOUT) ;
        remove(i) ;
      }
      continue ;
    }

   /* client closed => exit */
    if ((x[0].revents | x[1].revents) & IOPAUSE_EXCEPT) break ;

   /* client reading => flush pending data */
    if (x[1].revents & IOPAUSE_WRITE)
      if ((bufalloc_flush(bufalloc_1) == -1) && !error_isagain(errno))
      {
        cleanup() ;
        strerr_diefu1sys(111, "flush stdout") ;
      }
    if (x[2].revents & IOPAUSE_WRITE)
      if ((bufalloc_flush(&asyncout) == -1) && !error_isagain(errno))
      {
        cleanup() ;
        strerr_diefu1sys(111, "flush asyncout") ;
      }

   /* scan children for successes */
    for (i = 0 ; i < genalloc_len(s6lockio_t, &a) ; i++)
    {
      register s6lockio_t_ref p = genalloc_s(s6lockio_t, &a) + i ;
      if (p->p[0] < 0) continue ;
      if (x[p->xindex].revents & IOPAUSE_READ)
      {
        char c ;
        register int r = sanitize_read(fd_read(p->p[0], &c, 1)) ;
        if (!r) continue ;
        if (r < 0)
        {
          trig(p->id, errno) ;
          remove(i--) ;
        }
        else if (c != '!')
        {
          trig(p->id, EPROTO) ;
          remove(i--) ;
        }
        else
        {
          trig(p->id, 0) ;
          p->limit = taia_zero ;
        }
      }
    }

   /* signals arrived => handle them */
    if (x[3].revents & (IOPAUSE_READ | IOPAUSE_EXCEPT)) handle_signals() ;

   /* client writing => get data and parse it */
    if (buffer_len(buffer_0small) || x[0].revents & IOPAUSE_READ)
    {
      for (;;)
      {
        uint16 id ;
        register int r = netstring_get(buffer_0small, &indata, &instate) ;
        if (!r) break ;
        else if (r < 0)
        {
          r = netstring_okeof(buffer_0small, instate) ;
          cleanup() ;
          if (r) goto end ; /* normal exit */
          else strerr_diefu1sys(111, "read a netstring") ;
        }
        if (indata.len < 3)
        {
          cleanup() ;
          strerr_dief1x(100, "invalid client request") ;
        }
        uint16_unpack_big(indata.s, &id) ;
        switch (indata.s[2])  /* protocol parsing */
        {
          case '>' : /* release */
          {
            register unsigned int i = genalloc_len(s6lockio_t, &a) ;
            for (; i ; i--) if (genalloc_s(s6lockio_t, &a)[i-1].id == id) break ;
            if (i)
            {
              remove(i-1) ;
              answer(0) ;
            }
            else answer(ENOENT) ;
            break ;
          }
          case '<' : /* lock path */
          {
            s6lockio_t f = S6LOCKIO_ZERO ;
            char const *cargv[3] = { S6LOCKD_HELPER_PROG, 0, 0 } ;
            char const *cenvp[2] = { 0, 0 } ;
            uint32 options, pathlen ;
            if (indata.len < 23)
            {
              answer(EPROTO) ;
              break ;
            }
            uint32_unpack_big(indata.s + 3, &options) ;
            taia_unpack(indata.s + 7, &f.limit) ;
            uint32_unpack_big(indata.s + 19, &pathlen) ;
            if (pathlen + 23 != indata.len)
            {
              answer(EPROTO) ;
              break ;
            }
            f.id = id ;
            if (!stralloc_0(&indata))
            {
              answer(errno) ;
              break ;
            }
            indata.s[21] = '.' ;
            indata.s[22] = '/' ;
            cargv[1] = (char const *)indata.s + 21 ;
            if (options & S6LOCK_OPTIONS_EX) cenvp[0] = "S6LOCK_EX=1" ;
            f.pid = child_spawn(cargv[0], cargv, cenvp, 0, 0, f.p) ;
            if (!f.pid)
            {
              answer(errno) ;
              break ;
            }
            if (!genalloc_append(s6lockio_t, &a, &f))
            {
              s6lockio_free(&f) ;
              answer(errno) ;
              break ;
            }
            answer(0) ;
            break ;
          }
          default :
          {
            cleanup() ;
            strerr_dief1x(100, "invalid client request") ;
          }
        }
        indata.len = 0 ;
      } /* end loop: parse input from client */
    } /* end if: stuff to read on stdin */
  } /* end loop: main iopause */
 end:
  return 0 ;
}
