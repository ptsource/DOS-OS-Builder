/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "implode.h"
#include "ziperr.h"
#define ZE_MAP(err) \
        ( ((err) == IM_OK)      ? ZE_OK \
        : ((err) == IM_NOMEM)   ? ZE_MEM \
        : ((err) == IM_IOERR)   ? ZE_TEMP \
                                : (fprintf(stderr,"\nZE_MAP(%d)",(err)), \
                                    ZE_LOGIC))
#define IMP_SETUP       0       /* need to do "imp_setup" */
#define IMP_P1          1       /* setup done, ready for Pass 1 */
#define IMP_P2          2       /* Pass 1 done, ready for Pass 2 */
#define IMP_CLEAR       3       /* Pass 2 done, ready for "imp_clear" */
local int imp_state = IMP_SETUP;
FDATA fd;
int
imp_setup (filesize, pack_level)
    long filesize;  /* input file size */
    int pack_level; /* 0 = best speed, 9 = best compression, other = default */
{
    ImpErr retcode;
    extern char *tempname();
    if (imp_state != IMP_SETUP)
    {   fprintf (stderr, "\nimp_setup called with wrong state %d",
                 imp_state);
        return ZE_LOGIC;
    }
    imp_state = IMP_P1;
    fd.fd_bufsize = 8192;
    if (filesize < 5632) fd.fd_bufsize = 4096;
    fd.fd_strsize = 320;
    fd.fd_nbits   = (fd.fd_bufsize == 4096) ? 6 : 7;
    fd.fd_temp = topen ('I');
    if (fd.fd_temp == NULL)
        return ZE_MEM;
    retcode = lm_init (pack_level);
    if (retcode != IM_OK) return ZE_MAP (retcode);
    retcode = ct_init ();
    return ZE_MAP (retcode);
}
int
imp_p1 (buf, count)
    char *buf;                  /* input characters */
    int   count;                /* character count */
{   ImpErr retcode;
    if (imp_state != IMP_P1)
    {   fprintf (stderr, "\nimp_p1 called with wrong state %d",
                 imp_state);
        return ZE_LOGIC;
    }
    if (buf == NULL || count < 0)
    {   fprintf (stderr, "\nimp_p1 called with bad arguments");
        return ZE_LOGIC;
    }
    retcode = lm_input ((U_CHAR *) buf, (U_INT) count);
    return ZE_MAP (retcode);
}
int
imp_size (size, opts)
    long *size;                 /* imploded size */
    char *opts;                 /* implosion option info */
{   ImpErr retcode;
    if (imp_state != IMP_P1)
    {   fprintf (stderr, "\nimp_size called with wrong state %d",
                 imp_state);
        return ZE_LOGIC;
    }
    imp_state = IMP_P2;
    if (size == NULL || opts == NULL)
    {   fprintf (stderr, "\nimp_size called with bad arguments");
        return ZE_LOGIC;
    }
    retcode = lm_windup ();
    if (retcode != IM_OK) return ZE_MAP (retcode);
    retcode = ct_mktrees ();
    if (retcode != IM_OK) return ZE_MAP (retcode);
    *size = fd.fd_clen;
    *opts = (char)(((fd.fd_bufsize == 8192) ? 0x02 : 0)
          | ((fd.fd_method == LITERAL_TREE) ? 0x04 : 0));
    return ZE_OK;
}
int
imp_p2 (outfp)
    FILE *outfp;                        /* output (ZIP) file */
{   ImpErr retcode;
    if (imp_state != IMP_P2)
    {   fprintf (stderr, "\nimp_p2 called with wrong state %d",
                 imp_state);
        return ZE_LOGIC;
    }
    imp_state = IMP_CLEAR;
    if (outfp == NULL)
    {   fprintf (stderr, "\nimp_p2 called with bad arguments");
        return ZE_LOGIC;
    }
    retcode = ct_wrtrees (outfp);
    if (retcode != IM_OK) return ZE_MAP (retcode);
    retcode = ct_wrdata (outfp);
    if (retcode != IM_OK) return ZE_MAP (retcode);
    fflush (outfp);
    if (ferror (outfp)) return ZE_TEMP;
    return ZE_OK;
}
int
imp_clear ()
{   (void) lm_windup ();
    if (fd.fd_temp != NULL)
        (void) tclose (fd.fd_temp);
    (void) ct_windup ();
    imp_state = IMP_SETUP;
    return ZE_OK;
}
