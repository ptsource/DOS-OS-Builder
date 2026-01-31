/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "crypt.h"
#include "tempf.h"
#include <errno.h>
typedef long  L_INT;
typedef int   INT;
typedef short S_INT;
typedef unsigned long  UL_INT;
typedef unsigned int   U_INT;
typedef unsigned short US_INT;
typedef unsigned char  U_CHAR;
typedef unsigned long  CRC;
#define VOID void
#define local static            /* More meaningful outside functions */
#define TRUE  1
#define FALSE 0
typedef
enum
    {   IM_OK,                  /* all OK */
        IM_EOF,                 /* end of file on input */
        IM_IOERR,               /* I/O error */
        IM_BADARG,              /* invalid procedure argument */
        IM_NOMEM,               /* out of memory */
        IM_LOGICERR,            /* logic error */
        IM_NOCTBLS              /* no more code tables */
    }
    ImpErr;
typedef
enum
    {   NO_LITERAL_TREE,        /* use only two trees */
        LITERAL_TREE            /* use all three trees */
    }
    Method;
typedef
struct  fdata
    {   L_INT    fd_len;        /* # of bytes in file */
        L_INT    fd_clen;       /* compressed length */
        tFILE    *fd_temp;      /* temporary file stream pointer */
        U_INT    fd_bufsize;    /* size of sliding dictionary */
        U_INT    fd_strsize;    /* max string match length */
        U_INT    fd_nbits;      /* # distance bits to write literally */
        Method   fd_method;     /* compression method */
    }
    FDATA;
typedef
struct  match
    {   S_INT       ma_dist;    /* distance back into buffer */
        union {
           US_INT   ma_length;  /* length of matched string */
           U_CHAR   ma_litc[2]; /* literal characters matched */
        } l;
    }
    MATCH;
extern FDATA fd;                /* file data */
#ifndef MSDOS
extern int errno;               /* system error code */
#endif  /* MSDOS */
extern MATCH *ma_buf;           /* match info buffer */
#define MA_BUFSIZE 512
#ifdef  MODERN
#include <string.h>
#else
voidp *malloc();
char  *strcpy();
char  *strcat();
#endif  /* MODERN */
ImpErr  lm_init
        OF ((int pack_level));
ImpErr  lm_input
        OF ((U_CHAR *block, U_INT count));
ImpErr  lm_windup
        OF ((void));
ImpErr ct_init
        OF ((void));
ImpErr ct_tally
        OF ((MATCH *ma));
ImpErr ct_mktrees
        OF ((void));
ImpErr ct_wrtrees
        OF ((FILE *outfp));
ImpErr ct_wrdata
        OF ((FILE *outfp));
ImpErr ct_windup
        OF ((void));
ImpErr bi_init
        OF ((FILE *fp));
ImpErr bi_rlout
        OF ((int value, int length));
int bi_reverse
        OF ((int value, int length));
ImpErr bi_windup
        OF ((void));
int imp_setup
        OF ((long filesize, int pack_level));
int imp_p1
        OF ((char *buf, int count));
int imp_size
        OF ((long *size, char *opts));
int imp_p2
        OF ((FILE *outfp));
int imp_clear
        OF ((void));
