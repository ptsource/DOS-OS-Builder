/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#ifndef __unzip_h   /* prevent multiple inclusions */
#define __unzip_h
#if defined(__GO32__) && defined(unix)   /* MS-DOS extender:  NOT Unix */
#  undef unix
#endif
#if defined(unix) || defined(M_XENIX) || defined(COHERENT) || defined(__hpux)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* unix || M_XENIX || COHERENT || __hpux */
#if defined(__convexc__) || defined(MINIX)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* __convexc__ || MINIX */
#ifdef __COMPILER_KCC__
#  include <c-env.h>
#  ifdef SYS_T20
#    define TOPS20
#  endif
#endif /* __COMPILER_KCC__ */
#ifdef __POWERC
#  define __TURBOC__
#  define MSDOS
#endif /* __POWERC */
#if defined(__MSDOS__) && (!defined(MSDOS))   /* just to make sure */
#  define MSDOS
#endif
#if defined(linux) && (!defined(LINUX))
#  define LINUX
#endif
#if defined(__STDC__) || defined(MSDOS) || defined(sgi)
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if defined(__IBMC__) || defined(__EMX__) || defined(__WATCOMC__)
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if defined(THINK_C) || defined(MPW) || defined(WIN32) || defined(_SEQUENT_)
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if defined(ATARI_ST) || defined(__BORLANDC__)  /* || defined(__convexc__) */
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if defined(NOPROTO) && defined(PROTO)
#  undef PROTO
#endif
#ifdef PROTO
#  define OF(a) a
#else /* !PROTO */
#  define OF(a) ()
#endif /* ?PROTO */
#if defined(M_XENIX) || defined(DNIX)
#  define NO_STDDEF_H
#endif
#if (defined(__NetBSD__) && !defined(__386BSD__))
#  define __386BSD__
#endif
#if 0  /* GRR 931003:  BOGUS!  Screwed-up systems can define this explicitly */
#if defined(__GNUC__)
#  if (!defined(__EMX__) && !defined(__386BSD__) && !defined(LINUX))
#    define NO_STDLIB_H
#  endif
#endif /* __GNUC__ */
#endif /* 0 */
#if defined(apollo)          /* defines __STDC__ */
#    define NO_STDLIB_H
#endif /* apollo */
#ifdef DNIX
#  define SYSV
#  define SHORT_NAMES         /* 14-char limitation on path components */
#  define FILENAME_MAX  NAME_MAX    /* GRR:  experiment */
#endif
#if (defined(__SYSTEM_FIVE) || defined(M_SYSV) || defined(M_SYS5))
#  ifndef SYSV
#    define SYSV
#  endif /* !SYSV */
#endif /* __SYSTEM_FIVE || M_SYSV || M_SYS5 */
#if (defined(SYSV) || defined(CRAY) || defined(LINUX))
#  ifndef TERMIO
#    define TERMIO
#  endif /* !TERMIO */
#endif /* SYSV || CRAY || LINUX */
#if (defined(ultrix) || defined(bsd4_2) || defined(sun) || defined(pyr))
#  if (!defined(BSD) && !defined(SYSV))
#    define BSD
#  endif
#endif /* ultrix || bsd4_2 || sun || pyr */
#if defined(__convexc__) || defined(__386BSD__)
#  if (!defined(BSD) && !defined(SYSV))
#    define BSD
#  endif
#endif /* __convexc__ || __386BSD__ */
#ifdef pyr  /* Pyramid */
#  ifdef BSD
#    define pyr_bsd
#  endif
#  define ZMEM            /* should ZMEM only be for BSD universe...? */
#  define DECLARE_ERRNO   /*  (AT&T memcpy was claimed to be very slow) */
#endif /* pyr */
#if (defined(CRAY) && defined(ZMEM))
#  undef ZMEM
#endif
#if (defined(__TURBOC__) || defined(__WATCOMC__) || defined(VMS))
#  define WILD_STAT_BUG
#endif
#if (defined(__MINT__))
#  define WILD_STAT_BUG
#endif
#ifdef WILD_STAT_BUG
#  define SSTAT(path,pbuf) (iswild(path) || stat(path,pbuf))
#else
#  define SSTAT stat
#endif
#define STRNICMP zstrnicmp
#if 0  /* internal version renamed to zstrnicmp and used for all systems now */
#if (defined(SYSV) || defined(sun386) || defined(VMS) || defined(__GO32__))
#  define NO_STRNICMP
#endif
#if (defined(COHERENT))
#  define NO_STRNICMP
#endif
#ifndef NO_STRNICMP
#  if (!defined(MODERN) || defined(NeXT) || defined(CRAY) || defined(LINUX))
#    define strnicmp strncasecmp
#  endif
#  if (defined(__386BSD__) || defined(__bsdi__))
#    define strnicmp strncasecmp
#  endif
#endif
#endif /* 0 */
#ifndef MINIX            /* Minix needs it after all the other includes (?) */
#  include <stdio.h>
#endif
#include <ctype.h>       /* skip for VMS, to use tolower() function? */
#include <errno.h>       /* used in mapname() */
#include <string.h>      /* GRR:  EXPERIMENTAL! */
#ifdef MODERN
#  include <limits.h>    /* GRR:  EXPERIMENTAL!  (can be deleted) */
#endif
#if 0    /* GRR:  MORE EXPERIMENTING (moved to OS sections) */
#ifdef VMS
#  include <types.h>     /* (placed up here instead of in VMS section below */
#  include <stat.h>      /* because types.h is used in some other headers) */
#else /* !VMS */
#    if (!defined(THINK_C) && !defined(MPW) && !defined(ATARI_ST) && !defined(AZTEC_C))
#      include <sys/types.h>         /* off_t, time_t, dev_t, ... */
#      include <sys/stat.h>
#    endif /* !THINK_C && !MPW && !ATARI_ST && !AZTEC_C */
#endif /* ?VMS */
#endif
#ifdef EFT
#  define LONGINT off_t  /* Amdahl UTS nonsense ("extended file types") */
#else
#  define LONGINT long
#endif
#ifdef MODERN
#  ifndef NO_STDDEF_H
#    include <stddef.h>
#  endif
#  ifndef NO_STDLIB_H
#    include <stdlib.h>    /* standard library prototypes, malloc(), etc. */
#  endif
   typedef size_t extent;
   typedef void voidp;
#else /* !MODERN */
   LONGINT lseek();
   char *malloc();
   typedef unsigned int extent;
   typedef char voidp;
#  define void int
#endif /* ?MODERN */
#include <signal.h>      /* used in unzip.c, file_io.c */
#ifdef AMIGA
#  include "amiga/amiga.h"
#  ifndef AZTEC_C
#    include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#    include <sys/stat.h>
#  endif
#  include <fcntl.h>            /* O_BINARY for open() w/o CR/LF translation */
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif
#if (defined(ATARI_ST) || defined(__MINT__))    /* comments by [cjh] */
#  ifdef __TURBOC__
#    include <ext.h>   /* stat() */
#    include <tos.h>   /* OS-specific functions (Fdup) */
#  endif
#  include <time.h>
#  ifndef __MINT__
#    define dup     Fdup
#    define mkdir   Dcreate
#    define DIR_END '\\'
#  else
#    include <sys/types.h>     /* didn't we include this already? */
#    include <sys/stat.h>
#    include <fcntl.h>         /* O_BINARY */
#    include <unistd.h>        /* dup proto & unix system calls live here */
#    include <time.h>
#    include <stdio.h>         /* didn't we include this already? */
#    define dup dup
#    define mkdir mkdir
#    define DIR_END '/'        /* much sexier than DOS filenames... */
#    define timezone _timezone /* oops */
#    define DIRENT
#    define SYMLINKS
#    ifdef S_ISLNK             /* WARNING:  horrible kludge!!!! */
#      undef S_ISLNK           /* MiNTlibs <= pl.41 have symlinks wrong! */
#      define S_ISLNK(a) (((a) & 0xa000) == 0xa000)
#    endif                     /* remove this when it's fixed! */
#    ifdef SHORT_NAMES         /* library will truncate weird names on TOS fs */
#      undef SHORT_NAMES
#    endif
#    ifndef ZMEM
#      define ZMEM             /* we have bcopy, etc., ifndef __STRICT_ANSI__ */
#    endif
#  endif /* __MINT__ */
#  ifndef S_IFMT
#    define S_IFMT     (S_IFCHR | S_IFREG | S_IFDIR)
#  endif
#  define DATE_FORMAT  DF_MDY
#  ifndef lenEOL
#    define lenEOL 2           /* DOS convention; MiNT doesn't really care */
#  endif
#  ifndef PutNativeEOL
#    define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#  endif
#endif
#ifdef THINK_C
#  define MACOS
#  ifndef __STDC__            /* if Think C hasn't defined __STDC__ ... */
#    define __STDC__ 1        /*   make sure it's defined: it needs it */
#  else /* __STDC__ defined */
#    if !__STDC__             /* sometimes __STDC__ is defined as 0; */
#      undef __STDC__         /*   it needs to be 1 or required header */
#      define __STDC__ 1      /*   files are not properly included. */
#    endif /* !__STDC__ */
#  endif /* ?defined(__STDC__) */
#  define CREATOR       'KAHL'
#endif /* THINK_C */
#ifdef MPW
#  define MACOS
#  include <Errors.h>
#  include <Files.h>
#  include <Memory.h>
#  include <Quickdraw.h>
#  include <ToolUtils.h>
#  ifdef fileno
#    undef fileno
#  endif
#  ifdef MCH_MACINTOSH
#    define CREATOR     'Manx'
#  else
#    define CREATOR     'MPS '
#  endif
#endif /* MPW */
#ifdef MACOS
#  include <fcntl.h>            /* O_BINARY for open() w/o CR/LF translation */
#  define fileno(x)     ((x) == stdout ? 1 : (short)(x))
#  define open(x,y)     macopen((x), (y), gnVRefNum, glDirID)
#  define fopen(x,y)    macfopen((x), (y), gnVRefNum, glDirID)
#  define close         macclose
#  define fclose(x)     macclose(fileno((x)))
#  define read          macread
#  define write         macwrite
#  define lseek         maclseek
#  define creat(x,y)    maccreat((x), gnVRefNum, glDirID, gostCreator, gostType)
#  define stat(x,y)     macstat((x), (y), gnVRefNum, glDirID)
#  define dup
#  ifndef MCH_MACINTOSH
#    define NO_STRNICMP
#  endif
#  define DIR_END ':'
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(CR);
#  define MALLOC_WORK
#  ifdef THINK_C
#    define fgets       wfgets
#    define fflush(f)
#    define fprintf     wfprintf
#    define fputs(s,f)  wfprintf((f), "%s", (s))
#    define printf      wprintf
#    ifdef putc
#      undef putc
#    endif
#    define putc(c,f)   wfprintf((f), "%c", (c))
#    define getenv      macgetenv
#  endif
#  ifndef isascii
#    define isascii(c)  ((unsigned char)(c) <= 0x3F)
#  endif
#  include "macstat.h"
#  include "macdir.h"
#  ifdef CR
#    undef  CR
#  endif
typedef struct _ZipExtraHdr {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
} ZIP_EXTRA_HEADER;
typedef struct _MacInfoMin {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
    unsigned long signature;  /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    unsigned long lCrDat;     /*    4 bytes */
    unsigned long lMdDat;     /*    4 bytes */
    unsigned long flags ;     /*    4 bytes */
    unsigned long lDirID;     /*    4 bytes */
} MACINFOMIN;                 /* = 40 bytes for size of data */
typedef struct _MacInfo {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
    unsigned long signature;  /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    unsigned long lCrDat;     /*    4 bytes */
    unsigned long lMdDat;     /*    4 bytes */
    unsigned long flags ;     /*    4 bytes */
    unsigned long lDirID;     /*    4 bytes */
    char rguchVolName[28];    /*   28 bytes */
} MACINFO;                    /* = 68 bytes for size of data */
#endif /* MACOS */
#ifdef MSDOS
#  include <dos.h>           /* for REGS macro (TC) or _dos_setftime (MSC) */
#  ifdef __TURBOC__          /* includes Power C */
#    include <sys/timeb.h>   /* for structure ftime */
#    ifndef __BORLANDC__     /* there appears to be a bug (?) in Borland's */
#      include <mem.h>       /*  MEM.H related to __STDC__ and far poin-   */
#    endif                   /*  ters. (dpk)  [mem.h included for memcpy]  */
#  endif
#endif /* MSDOS */
#ifdef __IBMC__
#  define S_IFMT 0xF000
#  define timezone _timezone
#  define PIPE_ERROR (errno == EERRSET || errno == EOS2ERR)
#endif
#ifdef __WATCOMC__
#  define __32BIT__
#  undef far
#  define far
#  undef near
#  define near
#  define PIPE_ERROR (errno == -1)
#endif
#ifdef __EMX__
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#  define far
#endif
#ifdef __GO32__              /* note: MS-DOS compiler, not OS/2 */
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#  include <sys/timeb.h>     /* for structure ftime */
   int setmode(int, int);    /* not in djgpp's include files */
#endif
#if defined(_MSC_VER) && (!defined(MSC))
#  define MSC                /* for old versions, MSC must be set explicitly */
#endif
#if 0  /* GRR 930907:  MSC 5.1 does declare errno but doesn't define _MSC_VER */
#ifdef MSC
#  ifndef _MSC_VER           /* new with 5.1 or 6.0 ... */
#    define DECLARE_ERRNO    /* not declared in errno.h in 5.0 or earlier? */
#  endif
#endif
#endif /* 0 */
#if defined(MSDOS) || defined(OS2)
#  include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  include <time.h>           /* localtime() */
#  include <fcntl.h>          /* O_BINARY for open() w/o CR/LF translation */
#  define DIR_END '\\'
#  if (defined(M_I86CM) || defined(M_I86LM))
#    define MED_MEM
#  endif
#  if (defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__))
#    define MED_MEM
#  endif
#  ifndef __32BIT__
#    ifndef MED_MEM
#      define SMALL_MEM
#    endif
#    define USE_FWRITE        /* write() cannot write more than 32767 bytes */
#  endif
#  define DATE_FORMAT   dateformat()
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#endif
#ifdef OS2                    /* defined for all OS/2 compilers */
#  ifdef MSDOS
#    undef MSDOS
#  endif
#  ifdef isupper
#    undef isupper
#  endif
#  ifdef tolower
#    undef tolower
#  endif
#  define isupper(x)   IsUpperNLS((unsigned char)(x))
#  define tolower(x)   ToLowerNLS((unsigned char)(x))
#endif
#if defined(MSWIN) && defined(FILE_IO_C)
#  include "wizunzip.h"
#endif
#ifdef MTS
#  include <sys/types.h>    /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <sys/file.h>     /* MTS uses this instead of fcntl.h */
#  include <timeb.h>
#  include <time.h>
#  include <unix.h>         /* some important non-ANSI routines */
#  define mkdir(s,n) (-1)   /* no "make directory" capability */
#  define EBCDIC            /* set EBCDIC conversion on */
#  define NO_STRNICMP       /* unzip's is as good the one in MTS */
#  define USE_FWRITE
#  define close_outfile()  fclose(outfile)   /* can't set time on files */
#  define umask(n)            /* Don't have umask() on MTS */
#  define FOPWT         "w"   /* Open file for writing in TEXT mode */
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif
#ifdef WIN32  /* NT */
#  include <sys/types.h>        /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <io.h>               /* read(), open(), etc. */
#  include <time.h>
#  include <memory.h>
#  include <direct.h>           /* mkdir() */
#  include <fcntl.h>
#  if defined(FILE_IO_C)
#    include <conio.h>
#    include <sys\types.h>
#    include <sys\utime.h>
#    include <windows.h>
#  endif
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#  define NT
#endif
#ifdef TOPS20
#  include <sys/types.h>        /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  include <sys/param.h>
#  include <sys/time.h>
#  include <sys/timeb.h>
#  include <sys/file.h>
#  include <timex.h>
#  include <monsym.h>           /* get amazing monsym() macro */
   extern int open(), close(), read();
   extern int stat(), unlink(), jsys(), fcntl();
   extern long lseek(), dup(), creat();
#  define strchr    index       /* GRR: necessary? */
#  define strrchr   rindex
#  define REALLY_SHORT_SYMS
#  define NO_MKDIR
#  define DIR_BEG  '<'
#  define DIR_END  '>'
#  define DIR_EXT  ".directory"
#  define DATE_FORMAT  DF_MDY
#endif /* TOPS20 */
#ifdef UNIX
#  include <sys/types.h>       /* off_t, time_t, dev_t, ... */
#  include <sys/stat.h>
#  ifndef COHERENT
#    include <fcntl.h>         /* O_BINARY for open() w/o CR/LF translation */
#  else /* COHERENT */
#    ifdef _I386
#      include <fcntl.h>       /* Coherent 4.0.x, Mark Williams C */
#    else
#      include <sys/fcntl.h>   /* Coherent 3.10, Mark Williams C */
#    endif
#    define SHORT_SYMS
#    ifndef __COHERENT__       /* Coherent 4.2 has tzset() */
#      define tzset  settz
#    endif
#  endif /* ?COHERENT */
#  ifndef NO_PARAM_H
#    ifdef NGROUPS_MAX
#      undef NGROUPS_MAX       /* SCO bug:  defined again in <sys/param.h> */
#    endif
#    ifdef BSD
#      define TEMP_BSD         /* may be defined again in <sys/param.h> */
#      undef BSD
#    endif
#    include <sys/param.h>     /* conflict with <sys/types.h>, some systems? */
#    if defined(TEMP_BSD) && !defined(BSD)
#      define BSD
#      undef TEMP_BSD
#    endif
#  endif /* !NO_PARAM_H */
#  ifdef BSD
#    include <sys/time.h>
#    include <sys/timeb.h>
#    ifdef _AIX
#      include <time.h>
#    endif
#  else
#    include <time.h>
     struct tm *gmtime(), *localtime();
#  endif
#  if defined(__386BSD__) || defined(LINUX) || (defined(SYSV)&&defined(MODERN))
#    include <unistd.h>        /* this includes utime.h, at least on SGIs */
#  endif
#  if defined(__386BSD__) ||defined(_POSIX_SOURCE) ||defined(sgi)||defined(_AIX)
#    include <utime.h>   /* NeXT, at least, does NOT define utimbuf in here */
#  else
     struct utimbuf {
         time_t actime;        /* new access time */
         time_t modtime;       /* new modification time */
     };
#  endif /* ?(__386BSD__ || _POSIX_SOURCE || sgi || _AIX) */
#  if (defined(V7) || defined(pyr_bsd))
#    define strchr   index
#    define strrchr  rindex
#  endif
#  ifdef V7
#    define O_RDONLY 0
#    define O_WRONLY 1
#    define O_RDWR   2
#  endif
#  ifdef MINIX
#    include <stdio.h>
#  endif
#  define DATE_FORMAT   DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif /* UNIX */
#ifdef VMS
#  include <types.h>              /* GRR:  experimenting... */
#  include <stat.h>
#  include <time.h>               /* the usual non-BSD time functions */
#  include <file.h>               /* same things as fcntl.h has */
#  include <rms.h>
#  define _MAX_PATH NAM$C_MAXRSS  /* to define FILNAMSIZ below */
#  define RETURN    return_VMS    /* VMS interprets return codes incorrectly */
#  define DIR_BEG  '['
#  define DIR_END  ']'
#  define DIR_EXT  ".dir"
#  define DATE_FORMAT  DF_MDY
#  define lenEOL        1
#  define PutNativeEOL  *q++ = native(LF);
#endif /* VMS */
#ifdef __human68k__    /* DO NOT DEFINE DOS_OS2 HERE!  If Human68k is so much */
#  include <time.h>    /*  like MS-DOS and/or OS/2, create DOS_HUM_OS2 macro! */
#  include <fcntl.h>
#  include <io.h>
#  include <conio.h>
#  include <jctype.h>
#  include <sys/stat.h>
#  define DATE_FORMAT  DF_YMD    /* Japanese standard */
#  define lenEOL        2
#  define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}
#endif
#define UNZIP
#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_VERSION       42   /* if OS-needed-to-extract is VMS:  can do */
#if defined(MSDOS) || defined(NT) || defined(OS2)
#  define DOS_NT_OS2
#endif
#if defined(MSDOS) || defined(OS2)
#  define DOS_OS2
#endif
#if defined(MSDOS) || defined(OS2) || defined(ATARI_ST)
#  define DOS_OS2_TOS
#endif
#if defined(MSDOS) || defined(ATARI_ST)
#  define DOS_TOS
#endif
#if defined(MSDOS) || defined(TOPS20) || defined(VMS)
#  define DOS_T20_VMS
#endif
#if defined(TOPS20) || defined(VMS)
#  define T20_VMS
#endif
#if (!defined(MSDOS) && !defined(__IBMC__)) || defined(NT)
#  define near
#  define far
#endif
#if defined(__GO32__) || defined(__EMX__)
#  define near
#  define far
#endif
#ifndef DIR_END
#  define DIR_END '/'       /* last char before program name (or filename) */
#endif
#ifndef RETURN
#  define RETURN  return    /* only used in main() */
#endif
#define DIR_BLKSIZ  64      
#ifndef WSIZE
#  define WSIZE     0x8000  /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */
#ifndef INBUFSIZ
#  define INBUFSIZ  0x0800  /* 2K:  works for MS-DOS small model */
#endif
#ifdef SMALL_MEM          /* i.e., 16-bit OS's:  MS-DOS, OS/2 1.x, etc. */
#  define NO_ZIPINFO      /* GRR:  true until move all strings to far memory */
#  define OUTBUFSIZ INBUFSIZ
#  if (lenEOL == 1)
#    define RAWBUFSIZ (OUTBUFSIZ>>1)
#  else
#    define RAWBUFSIZ ((OUTBUFSIZ>>1) - (OUTBUFSIZ>>7))
#  endif
#  define TRANSBUFSIZ (OUTBUFSIZ-RAWBUFSIZ)
#else
#  ifdef MED_MEM
#    define OUTBUFSIZ 0xFF80     /* can't malloc arrays of 0xFFE8 or more */
#    define TRANSBUFSIZ 0xFF80
#  else
#    define OUTBUFSIZ (lenEOL*WSIZE)  /* more efficient text conversion */
#    define TRANSBUFSIZ (lenEOL*OUTBUFSIZ)
#  endif
#  define RAWBUFSIZ OUTBUFSIZ
#endif /* ?SMALL_MEM */
#if (defined(SFX) && !defined(NO_ZIPINFO))
#  define NO_ZIPINFO
#endif
#ifndef O_BINARY
#  define O_BINARY  0
#endif
#ifndef PIPE_ERROR
#  define PIPE_ERROR (errno == EPIPE)
#endif
#ifdef VMS
#  define FOPR  "r","ctx=stm"
#  define FOPM  "r+","ctx=stm","rfm=fix","mrs=512"
#  define FOPW  "w","ctx=stm","rfm=fix","mrs=512"
#else /* !VMS */
#  if defined(MODERN) || defined(AMIGA)
#    define FOPR "rb"
#    define FOPM "r+b"
#    ifdef TOPS20          /* TOPS-20 MODERN?  You kidding? */
#      define FOPW "w8"
#    else
#      define FOPW "wb"
#    endif
#  else /* !MODERN */
#    define FOPR "r"
#    define FOPM "r+"
#    define FOPW "w"
#  endif /* ?MODERN */
#endif /* VMS */
#ifdef DOS_OS2_TOS
#  include <limits.h>
#endif
#ifndef PATH_MAX
#  ifdef MAXPATHLEN
#    define PATH_MAX      MAXPATHLEN    /* in <sys/param.h> on some systems */
#  else
#    ifdef _MAX_PATH
#      define PATH_MAX    _MAX_PATH
#    else
#      if FILENAME_MAX > 255
#        define PATH_MAX  FILENAME_MAX  /* used like PATH_MAX on some systems */
#      else
#        define PATH_MAX  1024
#      endif
#    endif /* ?_MAX_PATH */
#  endif /* ?MAXPATHLEN */
#endif /* !PATH_MAX */
#define FILNAMSIZ  (PATH_MAX+1)
#ifdef SHORT_SYMS                   /* Mark Williams C, ...? */
#  define extract_or_test_files     xtr_or_tst_files
#  define extract_or_test_member    xtr_or_tst_member
#endif
#ifdef REALLY_SHORT_SYMS            /* TOPS-20 linker:  first 6 chars */
#  define process_cdir_file_hdr     XXpcdfh
#  define process_local_file_hdr    XXplfh
#  define extract_or_test_files     XXxotf  /* necessary? */
#  define extract_or_test_member    XXxotm  /* necessary? */
#  define check_for_newer           XXcfn
#  define overwrite_all             XXoa
#  define process_all_files         XXpaf
#  define extra_field               XXef
#  define explode_lit8              XXel8
#  define explode_lit4              XXel4
#  define explode_nolit8            XXnl8
#  define explode_nolit4            XXnl4
#  define cpdist8                   XXcpdist8
#  define inflate_codes             XXic
#  define inflate_stored            XXis
#  define inflate_fixed             XXif
#  define inflate_dynamic           XXid
#  define inflate_block             XXib
#  define maxcodemax                XXmax
#endif
#define ZSUFX             ".zip"
#define CENTRAL_HDR_SIG   "\113\001\002"   /* the infamous "PK" signature */
#define LOCAL_HDR_SIG     "\113\003\004"   /*  bytes, sans "P" (so unzip */
#define END_CENTRAL_SIG   "\113\005\006"   /*  executable not mistaken for */
#define EXTD_LOCAL_SIG    "\113\007\010"   /*  zipfile itself) */
#define SKIP              0    /* choice of activities for do_string() */
#define DISPLAY           1
#define FILENAME          2
#define EXTRA_FIELD       3
#define DOES_NOT_EXIST    -1   /* return values for check_for_newer() */
#define EXISTS_AND_OLDER  0
#define EXISTS_AND_NEWER  1
#define ROOT              0    /* checkdir() extract-to path:  called once */
#define INIT              1    /* allocate buildpath:  called once per member */
#define APPEND_DIR        2    /* append a dir comp.:  many times per member */
#define APPEND_NAME       3    /* append actual filename:  once per member */
#define GETPATH           4    /* retrieve the complete path and free it */
#define END               5    /* free root path prior to exiting program */
#define FS_FAT_           0    /* filesystem used by MS-DOS, OS/2, NT */
#define AMIGA_            1
#define VMS_              2
#define UNIX_             3
#define VM_CMS_           4
#define ATARI_            5    /* what if it's a minix filesystem? [cjh] */
#define FS_HPFS_          6    /* filesystem used by OS/2, NT */
#define MAC_              7
#define Z_SYSTEM_         8
#define CPM_              9
#define TOPS20_           10
#define FS_NTFS_          11   /* filesystem used by Windows NT */
#define NUM_HOSTS         12   /* index of last system + 1 */
#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define NUM_METHODS       9    /* index of last method + 1 */
#define PK_OK             0    /* no error */
#define PK_COOL           0    /* no error */
#define PK_GNARLY         0    /* no error */
#define PK_WARN           1    /* warning error */
#define PK_ERR            2    /* error in zipfile */
#define PK_BADERR         3    /* severe error in zipfile */
#define PK_MEM            4    /* insufficient memory */
#define PK_MEM2           5    /* insufficient memory */
#define PK_MEM3           6    /* insufficient memory */
#define PK_MEM4           7    /* insufficient memory */
#define PK_MEM5           8    /* insufficient memory */
#define PK_NOZIP          9    /* zipfile not found */
#define PK_PARAM          10   /* bad or illegal parameters specified */
#define PK_FIND           11   /* no files found */
#define PK_DISK           50   /* disk full */
#define PK_EOF            51   /* unexpected EOF */
#define IZ_DIR            76   /* potential zipfile is a directory */
#define IZ_CREATED_DIR    77   /* directory created: set time and permissions */
#define IZ_VOL_LABEL      78   /* volume label, but can't set on hard disk */
#define DF_MDY            0    /* date format 10/26/91 (USA only) */
#define DF_DMY            1    /* date format 26/10/91 (most of the world) */
#define DF_YMD            2    /* date format 91/10/26 (a few countries) */
#define LREC_SIZE     26    /* lengths of local file headers, central */
#define CREC_SIZE     42    /*  directory headers, and the end-of-    */
#define ECREC_SIZE    18    /*  central-dir record, respectively      */
#define MAX_BITS      13                 /* used in unshrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */
#define LF      10    /* '\n' on ASCII machines; must be 10 due to EBCDIC */
#define CR      13    /* '\r' on ASCII machines; must be 13 due to EBCDIC */
#define CTRLZ   26    /* DOS & OS/2 EOF marker (used in file_io.c, vms.c) */
#ifdef EBCDIC
#  define native(c)   ebcdic[(c)]
#  define NATIVE      "EBCDIC"
#endif
#ifdef MPW
#  define FFLUSH(f)   putc('\n',f)
#else
#  define FFLUSH      fflush
#endif
#ifdef ZMEM     /* GRR:  THIS IS AN EXPERIMENT... (seems to work) */
#  undef ZMEM
#  define memcpy(dest,src,len)   bcopy(src,dest,len)
#  define memzero                bzero
#else
#  define memzero(dest,len)      memset(dest,0,len)
#endif
#ifdef VMS
#  define ENV_UNZIP     "UNZIP_OPTS"      /* name of environment variable */
#  define ENV_ZIPINFO   "ZIPINFO_OPTS"
#else /* !VMS */
#  define ENV_UNZIP     "UNZIP"
#  define ENV_ZIPINFO   "ZIPINFO"
#endif /* ?VMS */
#if !defined(QQ) && !defined(NOQQ)
#  define QQ
#endif
#ifdef QQ                         /* Newtware version:  no file */
#  define QCOND     (!qflag)      /*  comments with -vq or -vqq */
#else                             /* Bill Davidsen version:  no way to */
#  define QCOND     (which_hdr)   /*  kill file comments when listing */
#endif
#ifdef OLD_QQ
#  define QCOND2    (qflag < 2)
#else
#  define QCOND2    (!qflag)
#endif
#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#endif
#ifndef FALSE
#  define FALSE     0
#endif
#ifndef SEEK_SET
#  define SEEK_SET  0
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif
#if (defined(UNIX) && defined(S_IFLNK) && !defined(MTS))
#  define SYMLINKS
#  ifndef S_ISLNK
#    define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#  endif
#endif /* UNIX && S_IFLNK && !MTS */
#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef IS_VOLID
#  define IS_VOLID(m)  ((m) & 0x08)
#endif
typedef char              boolean;
typedef unsigned char     uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short    ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long     ulg;  /*  predefined on some systems) & match zip  */
typedef struct min_info {
    long offset;
    ulg compr_size;          /* compressed size (needed if extended header) */
    ulg crc;                 /* crc (needed if extended header) */
    int hostnum;
    unsigned file_attr;      /* local flavor, as used by creat(), chmod()... */
    unsigned encrypted : 1;  /* file encrypted: decrypt before uncompressing */
    unsigned ExtLocHdr : 1;  /* use time instead of CRC for decrypt check */
    unsigned textfile : 1;   /* file is text (according to zip) */
    unsigned textmode : 1;   /* file is to be extracted as text */
    unsigned lcflag : 1;     /* convert filename to lowercase */
    unsigned vollabel : 1;   /* "file" is an MS-DOS volume (disk) label */
} min_info;
typedef struct VMStimbuf {
    char *revdate;           /* (both correspond to Unix modtime/st_mtime) */
    char *credate;
} VMStimbuf;
#ifdef MALLOC_WORK
   union work {
     struct {
       short *Prefix_of;            /* (8193 * sizeof(short)) */
       uch *Suffix_of;
       uch *Stack;
     } shrink;                      /* unshrink() */
     uch *Slide;                    /* explode(), inflate(), unreduce() */
   };
#else /* !MALLOC_WORK */
   union work {
     struct {
#ifdef HSIZE2    /* needed to avoid errors on some machines? */
       short Prefix_of[HSIZE + 2];  /* (8194 * sizeof(short)) */
       uch Suffix_of[HSIZE + 2];    /* also s-f length_nodes (smaller) */
       uch Stack[HSIZE + 2];        /* also s-f distance_nodes (smaller) */
#else /* !HSIZE2 */
       short Prefix_of[HSIZE];      /* (8192 * sizeof(short)) */
       uch Suffix_of[HSIZE];        /* also s-f length_nodes (smaller) */
       uch Stack[HSIZE];            /* also s-f distance_nodes (smaller) */
#endif /* ?HSIZE2 */
     } shrink;
     uch Slide[WSIZE];
   };
#endif /* ?MALLOC_WORK */
#define prefix_of   area.shrink.Prefix_of
#define suffix_of   area.shrink.Suffix_of
#define stack       area.shrink.Stack
#define slide       area.Slide
   typedef uch   local_byte_hdr[ LREC_SIZE ];
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24
   typedef uch   cdir_byte_hdr[ CREC_SIZE ];
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38
   typedef uch   ec_byte_rec[ ECREC_SIZE+4 ];
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10
#      define SIZE_CENTRAL_DIRECTORY            12
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20
   typedef struct local_file_header {                 /* LOCAL */
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
   } local_file_hdr;
   typedef struct central_directory_file_header {     /* CENTRAL */
       uch version_made_by[2];
       uch version_needed_to_extract[2];
       ush general_purpose_bit_flag;
       ush compression_method;
       ush last_mod_file_time;
       ush last_mod_file_date;
       ulg crc32;
       ulg csize;
       ulg ucsize;
       ush filename_length;
       ush extra_field_length;
       ush file_comment_length;
       ush disk_number_start;
       ush internal_file_attributes;
       ulg external_file_attributes;
       ulg relative_offset_local_header;
   } cdir_file_hdr;
   typedef struct end_central_dir_record {            /* END CENTRAL */
       ush number_this_disk;
       ush num_disk_with_start_central_dir;
       ush num_entries_centrl_dir_ths_disk;
       ush total_entries_central_dir;
       ulg size_central_directory;
       ulg offset_start_central_directory;
       ush zipfile_comment_length;
   } ecdir_rec;
#ifndef __
#  define __   OF
#endif
int    uz_opts                   __((int *pargc, char ***pargv));
int    usage                     __((int error));
int    process_zipfiles          __((void));
int    do_seekable               __((int lastchance));
int    uz_end_central            __((void));
int    process_cdir_file_hdr     __((void));
int    process_local_file_hdr    __((void));
int    zi_opts                   __((int *pargc, char ***pargv));
int    zi_end_central            __((void));
int    zipinfo                   __((void));
ulg    SizeOfEAs                 __((void *extra_field));  /* also in os2.c? */
int    list_files                __((void));
int      open_input_file    __((void));
int      open_outfile       __((void));                        /* also vms.c */
int      readbuf            __((char *buf, register unsigned len));
int      FillBitBuffer      __((void));
int      readbyte           __((void));
#ifdef FUNZIP
   int   flush              __((ulg size));
#else
   int   flush              __((uch *buf, ulg size, int unshrink));
#endif
void     handler            __((int signal));
time_t   dos_to_unix_time   __((unsigned ddate, unsigned dtime));
int      check_for_newer    __((char *filename));       /* also os2.c, vms.c */
int      find_end_central_dir __((long searchlen));        /* find_ecrec */
int      get_cdir_file_hdr  __((void));                    /* get_cdir_ent */
int      do_string          __((unsigned int len, int option));
ush      makeword           __((uch *b));
ulg      makelong           __((uch *sig));
int      zstrnicmp __((register char *s1, register char *s2, register int n));
#ifdef ZMEM   /* MUST be ifdef'd because of conflicts with the standard def. */
   char *memset __((register char *, register char, register unsigned int));
   char *memcpy __((register char *, register char *, register unsigned int));
#endif
int    extract_or_test_files     __((void));
int    memextract                __((uch *, ulg, uch *, ulg));
int    FlushMemory               __((void));
int    ReadMemoryByte            __((ush *x));
int    explode                   __((void));                    /* explode.c */
int    inflate                   __((void));                    /* inflate.c */
int    inflate_free              __((void));                    /* inflate.c */
void   unreduce                  __((void));                   /* unreduce.c */
int    unshrink                  __((void));                   /* unshrink.c */
#ifdef __human68k__
   void     InitTwentyOne        __((void));
#endif
#ifdef MACOS
   int      macmkdir             __((char *, short, long));
   short    macopen              __((char *, short, short, long));
   FILE    *macfopen             __((char *, char *, short, long));
   short    maccreat             __((char *, short, long, OSType, OSType));
   short    macread              __((short, char *, unsigned));
   long     macwrite             __((short, char *, unsigned));
   short    macclose             __((short));
   long     maclseek             __((short, long, short));
   char    *wfgets               __((char *, int, FILE *));
   void     wfprintf             __((FILE *, char *, ...));
   void     wprintf              __((char *, ...));
#endif
#if (defined(__GO32__) || (defined(MSDOS) && defined(__EMX__)))
   void _dos_setftime(int, unsigned short, unsigned short);       /* msdos.c */
   void _dos_setfileattr(char *, int);                            /* msdos.c */
   unsigned _dos_creat(char *, unsigned, int *);                  /* msdos.c */
   void _dos_getdrive(unsigned *);                                /* msdos.c */
   unsigned _dos_close(int);                                      /* msdos.c */
#endif
#ifdef OS2  /* GetFileTime conflicts with something in NT header files */
   int   GetCountryInfo   __((void));                               /* os2.c */
   long  GetFileTime      __((char *name));                         /* os2.c */
   void  SetPathInfo      __((char *path, ush moddate, ush modtime, int flags));
   int   IsEA             __((void *extra_field));                  /* os2.c */
   void  SetEAs           __((char *path, void *eablock));          /* os2.c */
   ulg   SizeOfEAs        __((void *extra_field));                  /* os2.c */
   int   IsUpperNLS       __((int nChr));                           /* os2.c */
   int   ToLowerNLS       __((int nChr));                           /* os2.c */
   void  DebugMalloc      __((void));                               /* os2.c */
#endif
#ifdef TOPS20
   int upper               __((char *s));                        /* tops20.c */
   int enquote             __((char *s));                        /* tops20.c */
   int dequote             __((char *s));                        /* tops20.c */
   int fnlegal             __(());  /* error if prototyped(?) */ /* tops20.c */
#endif
#ifdef VMS
int      check_format      __((void));                              /* vms.c */
int      find_vms_attrs    __((void));                              /* vms.c */
int      CloseOutputFile   __((void));                              /* vms.c */
void     return_VMS        __((int zip_error));
#endif
int      match             __((char *s, char *p, int ic));        /* match.c */
int      iswild            __((char *p));                         /* match.c */
void     envargs           __((int *, char ***, char *));       /* envargs.c */
void     mksargs           __((int *, char ***));               /* envargs.c */
int      dateformat        __((void));
int      mapattr           __((void));                              /* local */
int      mapname           __((int renamed));                       /* local */
int      checkdir          __((char *pathcomp, int flag));          /* local */
char    *do_wild           __((char *wildzipfn));                   /* local */
#ifndef MTS /* macro in MTS */
   void  close_outfile     __((void));                              /* local */
#endif
#ifndef MAX
#  define MAX(a,b)   ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif
#ifdef DEBUG
#  define Trace(x)   fprintf x
#else
#  define Trace(x)
#endif
#if defined(UNIX) || defined(T20_VMS)   /* generally old systems */
#  define ToLower(x)   ((char)(isupper((int)x)? tolower((int)x) : x))
#else
#  define ToLower      tolower          /* assumed "smart"; used in match() */
#endif
#define LSEEK(abs_offset) {LONGINT request=(abs_offset)+extra_bytes,\
   inbuf_offset=request%INBUFSIZ, bufstart=request-inbuf_offset;\
   if(request<0) {fprintf(stderr, SeekMsg, ReportMsg); return(3);}\
   else if(bufstart!=cur_zipfile_bufstart)\
   {cur_zipfile_bufstart=lseek(zipfd,(LONGINT)bufstart,SEEK_SET);\
   if((incnt=read(zipfd,(char *)inbuf,INBUFSIZ))<=0) return(51);\
   inptr=inbuf+(int)inbuf_offset; incnt-=(int)inbuf_offset;} else\
   {incnt+=(inptr-inbuf)-(int)inbuf_offset; inptr=inbuf+(int)inbuf_offset;}}
#define SKIP_(length) if(length&&((error=do_string(length,SKIP))!=0))\
  {error_in_archive=error; if(error>1) return error;}
#ifdef FUNZIP
#  define FLUSH    flush
#  define NEXTBYTE getc(in)   /* redefined in crypt.h if full version */
#else
#  define FLUSH(w) if (mem_mode) outcnt=(w); else flush(slide,(ulg)w,0)
#  define NEXTBYTE (csize-- <= 0 ? EOF : (--incnt >= 0 ? *inptr++ : readbyte()))
#endif
#define READBITS(nbits,zdest) {if(nbits>bits_left) {int temp; zipeof=1;\
  while (bits_left<=8*(sizeof(bitbuf)-1) && (temp=NEXTBYTE)!=EOF) {\
  bitbuf|=(ulg)temp<<bits_left; bits_left+=8; zipeof=0;}}\
  zdest=(int)((ush)bitbuf&mask_bits[nbits]);bitbuf>>=nbits;bits_left-=nbits;}
#define NUKE_CRs(buf,len) \
{ \
    register int i, j; \
    for (i = j = 0;  j < len;  (buf)[i++] = (buf)[j++]) \
        if ((buf)[j] == CR) \
            ++j; \
    len = i; \
}
#ifdef __human68k__
#  define TOLOWER(str1, str2) \
   { \
       char *p=(str1), *q=(str2); \
       uch c; \
       while ((c = *p++) != '\0') { \
           if (iskanji(c)) { \
               if (*p == '\0') \
                   break; \
               *q++ = c; \
               *q++ = *p++; \
           } else \
               *q++ = isupper(c) ? tolower(c) : c; \
       } \
       *q = '\0'; \
   }
#else
#  define TOLOWER(str1, str2) \
   { \
       char  *p, *q; \
       p = (str1) - 1; \
       q = (str2); \
       while (*++p) \
           *q++ = (char)(isupper((int)(*p))? tolower((int)(*p)) : *p); \
       *q = '\0'; \
   }
#endif
#ifndef native
#  define native(c)   (c)
#  define A_TO_N(str1)
#else
#  ifndef NATIVE
#    define NATIVE     "native chars"
#  endif
#  define A_TO_N(str1) {register unsigned char *p;\
     for (p=str1; *p; p++) *p=native(*p);}
#endif
   extern int       zipinfo_mode;
   extern int       aflag;
   extern int       cflag;
   extern int       fflag;
   extern int       hflag;
   extern int       jflag;
   extern int       lflag;
   extern int       overwrite_none;
   extern int       overwrite_all;
   extern int       force_flag;
   extern int       qflag;
#ifdef DOS_NT_OS2
   extern int       sflag;
   extern int       volflag;
#endif
   extern int       tflag;
   extern int       U_flag;
   extern int       uflag;
   extern int       V_flag;
   extern int       vflag;
#ifdef VMS
   extern int       secinf;
#endif
   extern int       zflag;
#ifdef MACOS
   extern int       HFSFlag;
#endif
   extern int       filespecs;
   extern int       xfilespecs;
   extern int       process_all_files;
   extern int       create_dirs;
   extern LONGINT   real_ecrec_offset;
   extern LONGINT   expect_ecrec_offset;
   extern long      csize;
   extern long      ucsize;
   extern long      used_csize;
   extern char      **pfnames;
   extern char      **pxnames;
   extern char near sig[];
   extern char near answerbuf[];
   extern min_info  *pInfo;
   extern union work area;
   extern ulg near  crc_32_tab[];
   extern ulg       crc32val;
   extern ush near  mask_bits[];
   extern uch       *inbuf;
   extern uch       *inptr;
   extern int       incnt;
   extern ulg       bitbuf;
   extern int       bits_left;
   extern boolean   zipeof;
   extern char      *zipfn;
   extern int       zipfd;
   extern LONGINT   ziplen;
   extern LONGINT   cur_zipfile_bufstart;
   extern LONGINT   extra_bytes;
   extern uch       *extra_field;
   extern uch       *hold;
   extern char near local_hdr_sig[];
   extern char near central_hdr_sig[];
   extern char near end_central_sig[];
   extern local_file_hdr  lrec;
   extern cdir_file_hdr   crec;
   extern ecdir_rec       ecrec;
   extern struct stat     statbuf;
   extern int       mem_mode;
   extern int       disk_full;
   extern int       newfile;
#ifdef SYMLINKS
   extern int       symlnk;
#endif
#ifdef FUNZIP
   extern FILE      *in;
#endif
   extern FILE      *outfile;
   extern uch       *outbuf;
   extern uch       *outbuf2;
   extern uch       *outptr;
   extern ulg       outcnt;
#ifdef MSWIN
   extern char      *filename;
#else
   extern char near filename[];
#endif
   extern char      *EndSigMsg;
   extern char      *CentSigMsg;
   extern char      *SeekMsg;
   extern char      *ReportMsg;
#ifdef DECLARE_ERRNO
   extern int       errno;
#endif
#ifdef EBCDIC
   extern uch       ebcdic[];
#endif
#ifdef MACOS
   extern short     gnVRefNum;
   extern long      glDirID;
   extern OSType    gostCreator;
   extern OSType    gostType;
   extern boolean   fMacZipped;
   extern boolean   macflag;
   extern short     giCursor;
   extern CursHandle rghCursor[];
#endif
#endif /* !__unzip_h */
