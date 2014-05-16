/* ISC license. */

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
#include "tai.h"
#include "djbunix.h"
#include "iopause.h"
#include "netstring.h"
#include "sredfa.h"
#include "skaclient.h"
#include "ftrig1.h"
#include "ftrigr.h"

static bufalloc asyncout = BUFALLOC_ZERO ;

typedef struct ftrigio_s ftrigio_t, *ftrigio_t_ref ;
struct ftrigio_s
{
  unsigned int xindex ;
  ftrig1 trig ;
  struct sredfa *re ;
  uint32 dfastate ;
  uint32 options ;
  uint16 id ; /* given by client */
} ;
#define FTRIGIO_ZERO { 0, FTRIG1_ZERO, 0, SREDFA_START, 0, 0 }
static ftrigio_t const fzero = FTRIGIO_ZERO ;

static genalloc a = GENALLOC_ZERO ; /* array of ftrigio_t */

static void ftrigio_deepfree (ftrigio_t_ref p)
{
  ftrig1_free(&p->trig) ;
  sredfa_delete(p->re) ;
  *p = fzero ;
}

static void cleanup (void)
{
  register unsigned int i = genalloc_len(ftrigio_t, &a) ;
  for (; i ; i--) ftrigio_deepfree(genalloc_s(ftrigio_t, &a) + i - 1) ;
  genalloc_setlen(ftrigio_t, &a, 0) ;
}
 
static void trig (uint16 id, char what, char info)
{
  char pack[7] = "4:idwi," ;
  uint16_pack_big(pack+2, id) ;
  pack[4] = what ; pack[5] = info ;
  if (!bufalloc_put(&asyncout, pack, 7))
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
  register unsigned int n = genalloc_len(ftrigio_t, &a) - 1 ;
  ftrigio_deepfree(genalloc_s(ftrigio_t, &a) + i) ;
  genalloc_s(ftrigio_t, &a)[i] = genalloc_s(ftrigio_t, &a)[n] ;
  genalloc_setlen(ftrigio_t, &a, n) ;
}

int main (void)
{
  stralloc indata = STRALLOC_ZERO ;
  unsigned int instate = 0 ;
  PROG = "s6-ftrigrd" ;

  if (ndelay_on(0) < 0) strerr_diefu2sys(111, "ndelay_on ", "0") ;
  if (ndelay_on(1) < 0) strerr_diefu2sys(111, "ndelay_on ", "1") ;
  if (sig_ignore(SIGPIPE) < 0) strerr_diefu1sys(111, "ignore SIGPIPE") ;

  {
    struct taia deadline, stamp ;
    taia_now(&stamp) ;
    taia_addsec(&deadline, &stamp, 2) ;
    if (!skaserver2_sync(&asyncout, FTRIGR_BANNER1, FTRIGR_BANNER1_LEN, FTRIGR_BANNER2, FTRIGR_BANNER2_LEN, &deadline, &stamp))
      strerr_diefu1sys(111, "sync with client") ;
  }

  for (;;)
  {
    register unsigned int n = genalloc_len(ftrigio_t, &a) ;
    iopause_fd x[3 + n] ;
    unsigned int i = 0 ;
    int r ;

    x[0].fd = 0 ; x[0].events = IOPAUSE_EXCEPT | IOPAUSE_READ ;
    x[1].fd = 1 ; x[1].events = IOPAUSE_EXCEPT | (bufalloc_len(bufalloc_1) ? IOPAUSE_WRITE : 0) ;
    x[2].fd = bufalloc_fd(&asyncout) ; x[2].events = IOPAUSE_EXCEPT | (bufalloc_len(&asyncout) ? IOPAUSE_WRITE : 0) ;
    for (; i < n ; i++)
    {
      register ftrigio_t_ref p = genalloc_s(ftrigio_t, &a) + i ;
      p->xindex = 3 + i ;
      x[3+i].fd = p->trig.fd ;
      x[3+i].events = IOPAUSE_READ ;
    }

    r = iopause(x, 3 + n, 0, 0) ;
    if (r < 0)
    {
      cleanup() ;
      strerr_diefu1sys(111, "iopause") ;
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

   /* scan listening ftrigs */
    for (i = 0 ; i < genalloc_len(ftrigio_t, &a) ; i++)
    {
      register ftrigio_t_ref p = genalloc_s(ftrigio_t, &a) + i ;
      if (x[p->xindex].revents & IOPAUSE_READ)
      {
        char c ;
        register int r = sanitize_read(fd_read(p->trig.fd, &c, 1)) ;
        if (!r) continue ;
        if (r < 0)
        {
          trig(p->id, 'd', errno) ;
          remove(i--) ;
        }
        else if (!sredfa_feed(p->re, &p->dfastate, c))
        {
          trig(p->id, 'd', ENOEXEC) ;
          remove(i--) ;
        }
        else if (p->dfastate & SREDFA_ACCEPT)
        {
          trig(p->id, '!', c) ;
          if (p->options & FTRIGR_REPEAT)
            p->dfastate = SREDFA_START ;
          else remove(i--) ;
        }
      }
    }

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
          case 'U' : /* unsubscribe */
          {
            register unsigned int i = genalloc_len(ftrigio_t, &a) ;
            for (; i ; i--) if (genalloc_s(ftrigio_t, &a)[i-1].id == id) break ;
            if (i) remove(i-1) ;
            answer(0) ;
            break ;
          }
          case 'L' : /* subscribe to path and match re */
          {
            ftrigio_t f = FTRIGIO_ZERO ;
            uint32 pathlen, relen ;
            if (indata.len < 18)
            {
              answer(EPROTO) ;
              break ;
            }
            uint32_unpack_big(indata.s + 3, &f.options) ;
            uint32_unpack_big(indata.s + 7, &pathlen) ;
            uint32_unpack_big(indata.s + 11, &relen) ;
            if (((pathlen + relen + 16) != indata.len) || indata.s[15 + pathlen])
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
            f.re = sredfa_new() ;
            if (!f.re)
            {
              answer(errno) ;
              break ;
            }
            if (!sredfa_from_regexp(f.re, indata.s + 16 + pathlen)
             || !ftrig1_make(&f.trig, indata.s + 15))
            {
              sredfa_delete(f.re) ;
              answer(errno) ;
              break ;
            }
            if (!genalloc_append(ftrigio_t, &a, &f))
            {
              ftrigio_deepfree(&f) ;
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
