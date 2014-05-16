/* ISC license. */

#ifndef FTRIG1_H
#define FTRIG1_H

#include "stralloc.h"

#define FTRIG1_PREFIX "ftrig1"
#define FTRIG1_PREFIXLEN (sizeof FTRIG1_PREFIX - 1)

typedef struct ftrig1_s ftrig1, *ftrig1_ref ;
struct ftrig1_s
{
  int fd ;
  int fdw ;
  stralloc name ;
} ;
#define FTRIG1_ZERO { -1, -1, STRALLOC_ZERO }

extern int ftrig1_make (ftrig1_ref, char const *) ;
extern void ftrig1_free (ftrig1_ref) ;

#endif
