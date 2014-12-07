/* ISC license. */

#ifndef FTRIGR_H
#define FTRIGR_H

#include "skalibs-config.h"
#include "uint16.h"
#include "uint32.h"
#include "tai.h"
#include "genalloc.h"
#include "gensetdyn.h"
#include "skaclient.h"
#include "s6-config.h"


 /* Constants */

#ifdef SKALIBS_SPROOT
# define FTRIGR_IPCPATH SKALIBS_SPROOT "/service/ftrigrd/s"
#else
# define FTRIGR_IPCPATH "/tmp/.ftrigrd-socket"
#endif

#define FTRIGRD_PROG S6_BINPREFIX "s6-ftrigrd"
#define FTRIGR_BANNER1 "ftrigr v1.0 (b)\n"
#define FTRIGR_BANNER1_LEN (sizeof FTRIGR_BANNER1 - 1)
#define FTRIGR_BANNER2 "ftrigr v1.0 (a)\n"
#define FTRIGR_BANNER2_LEN (sizeof FTRIGR_BANNER2 - 1)


 /* Internals of the ftrigr_t */

typedef enum fr1state_e fr1state_t, *fr1state_t_ref ;
enum fr1state_e
{
  FR1STATE_WAITACK,
  FR1STATE_WAITACKDATA,
  FR1STATE_LISTENING,
  FR1STATE_ERROR
} ;
      
typedef struct ftrigr1_s ftrigr1_t, *ftrigr1_t_ref ;
struct ftrigr1_s
{
  uint32 options ;
  unsigned int count ;
  fr1state_t state ;
  char what ;
} ;
#define FTRIGR1_ZERO { 0, 0, FR1STATE_ERROR, 0 }
extern ftrigr1_t const ftrigr1_zero ;


 /* The ftrigr_t itself */

typedef struct ftrigr_s ftrigr, ftrigr_t, *ftrigr_ref, *ftrigr_t_ref ;
struct ftrigr_s
{
  skaclient2_t connection ;
  genalloc list ; /* array of uint16 */
  gensetdyn data ; /* set of ftrigr1_t */
} ;
#define FTRIGR_ZERO { .connection = SKACLIENT2_ZERO, .list = GENALLOC_ZERO, .data = GENSETDYN_INIT(ftrigr1_t, 2, 0, 1) }
extern ftrigr const ftrigr_zero ;


 /* Starting and ending a session */

extern int ftrigr_start (ftrigr_ref, char const *, struct taia const *, struct taia *) ;
#define ftrigr_start_g(a, path, deadline) ftrigr_start(a, path, (deadline), &STAMP)
extern int ftrigr_startf (ftrigr_ref, struct taia const *, struct taia *) ;
#define ftrigr_startf_g(a, deadline) ftrigr_startf(a, (deadline), &STAMP)
extern void ftrigr_end (ftrigr_ref) ;
                    

 /* Instant primitives for async programming */

#define ftrigr_fd(a) skaclient2_fd(&(a)->connection)
extern int ftrigr_update (ftrigr_ref) ;
extern int ftrigr_check (ftrigr_ref, uint16, char *) ;


 /* Synchronous functions with timeouts */

#define FTRIGR_REPEAT 0x0001

extern uint16 ftrigr_subscribe (ftrigr_ref, char const *, char const *, uint32, struct taia const *, struct taia *) ;
#define ftrigr_subscribe_g(a, path, re, options, deadline) ftrigr_subscribe(a, path, re, options, (deadline), &STAMP)
extern int ftrigr_unsubscribe (ftrigr_ref, uint16, struct taia const *, struct taia *) ;
#define ftrigr_unsubscribe_g(a, id, deadline) ftrigr_unsubscribe(a, id, (deadline), &STAMP)

extern int ftrigr_wait_and (ftrigr_ref, uint16 const *, unsigned int, struct taia const *, struct taia *) ;
#define ftrigr_wait_and_g(a, list, len, deadline) ftrigr_wait_and(a, list, len, (deadline), &STAMP)
extern int ftrigr_wait_or  (ftrigr_ref, uint16 const *, unsigned int, struct taia const *, struct taia *, char *) ;
#define ftrigr_wait_or_g(a, list, len, deadline, what) ftrigr_wait_or(a, list, len, deadline, &STAMP, what)

#endif
