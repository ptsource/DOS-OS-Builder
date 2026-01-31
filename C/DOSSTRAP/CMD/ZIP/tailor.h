/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#define const
#ifdef __STDC__
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* __STDC__ */
#ifdef sgi
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* sgi */
#ifdef __POWERC                 /* For Power C too */
#  define __TURBOC__
#endif /* __POWERC */
#ifdef __TURBOC__
#  ifndef MSDOS
#    define MSDOS
#  endif /* !MSDOS */
#endif /* __TURBOC__ */
#ifdef MSDOS
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif /* MSDOS */
#ifdef NOPROTO
#  ifdef PROTO
#    undef PROTO
#  endif /* PROTO */
#endif /* NOPROT */
#ifdef PROTO
#  define OF(a) a
#else /* !PROTO */
#  define OF(a) ()
#endif /* ?PROTO */
#ifdef MSDOS
#  ifdef __TURBOC__
#    include <alloc.h>
#  else /* !__TURBOC__ */
#    include <malloc.h>
#    define farmalloc _fmalloc
#    define farfree   _ffree
#  endif /* ?__TURBOC__ */
#else /* !MSDOS */
#  define huge
#  define far
#  define near
#  define farmalloc malloc
#  define farfree   free
#endif /* ?MSDOS */
#ifdef MSDOS
#  define MSVMS
#else /* !MSDOS */
#  ifdef VMS
#    define MSVMS
#  endif /* VMS */
#endif /* ?MSDOS */
#include <stdio.h>
#ifdef MODERN
#  ifndef M_XENIX
#    include <stddef.h>
#  endif /* !M_XENIX */
#  include <stdlib.h>
   typedef size_t extent;
   typedef void voidp;
#else /* !MODERN */
   typedef unsigned int extent;
#  define void int
   typedef char voidp;
#endif /* ?MODERN */
#ifdef VMS
#  include <types.h>
#  include <stat.h>
#else /* !VMS */
#  include <sys/types.h>
#  include <sys/stat.h>
#endif /* ?VMS */
#ifdef VMS
#  define unlink delete
#endif /* VMS */
#ifdef pyr
#  define strrchr rindex
#  define ZMEM
#endif /* pyr */
#ifdef MODERN
#  define FOPR "rb"
#  define FOPM "r+b"
#  define FOPW "w+b"
#else /* !MODERN */
#  define FOPR "r"
#  define FOPM "r+"
#  define FOPW "w+"
#endif /* ?MODERN */
#ifndef MSDOS
#   define BSZ 8192   /* Buffer size for files */
#else /* !MSDOS */
#   define BSZ 4096   /* Keep precious NEAR space */
#endif /* ?MSDOS */
