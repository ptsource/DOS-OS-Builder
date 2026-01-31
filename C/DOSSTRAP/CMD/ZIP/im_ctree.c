/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#ifdef DEBUG
#   define VALIDATE
#endif /* DEBUG */
#include "implode.h"
typedef
struct  ct_data
    {   UL_INT  ct_freq;                /* frequency count */
        US_INT  ct_code;                /* bit string */
        U_CHAR  ct_len;                 /* length of bit string */
        U_CHAR  ct_val;                 /* source value */
    }
    TRDATA;
typedef
struct  ct_resort
    {   U_CHAR  ct_rlen;                /* length of bit string */
        U_CHAR  ct_rval;                /* source value */
#ifdef VMS
        US_INT  dummy;                  /* because of bug in qsort() */
#endif /* VMS */
    }
    RESORT;
typedef
struct  ct_desc
    {   TRDATA *ct_array;               /* array of TRDATA */
        int     ct_size;                /* # of entries in tree */
    }
    TRDESC;
#define MAXTREES        5               /* max # of trees at once */
local   TRDESC  ct_table[MAXTREES];
#define VALID_HANDLE(x) \
        ((x) >= 0 && (x) < MAXTREES && ct_table[x].ct_array != NULL)
local   long    ct_litc_num;            /* total # literal chars */
local   long    ct_lit2_num;            /* total # of 2-char matches */
local   long    ct_strg_num;            /* total # of string matches */
local   long    ct_litc_freq[256];      /* literal character freqs */
local   long    ct_len2_freq[64];       /* length freqs (MML=2) */
local   long    ct_len3_freq[64];       /* length freqs (MML=3) */
local   long    ct_dst2_freq[64];       /* distance freqs (MML=2) */
local   long    ct_dst3_freq[64];       /* distance freqs (MML=3) */
local   long    ct_litc_saved;          /* literal tree */
local   long    ct_len2_saved;          /* length tree (MML=2) */
local   long    ct_len3_saved;          /* length tree (MML=3) */
local   long    ct_dst2_saved;          /* distance tree (MML=2) */
local   long    ct_dst3_saved;          /* distance tree (MML=3) */
local   int     ct_litc_tree;           /* temp literal tree */
local   int     ct_len2_tree;           /* temp length tree (MML=2) */
local   int     ct_len3_tree;           /* temp length tree (MML=3) */
local   int     ct_dst2_tree;           /* temp distance tree (MML=2) */
local   int     ct_dst3_tree;           /* temp distance tree (MML=3) */
local   int     lit_tree;               /* literal tree (-1 if none) */
local   int     len_tree;               /* length tree */
local   int     dst_tree;               /* distance tree */
local ImpErr ct_alloc
        OF ((int size, int *handle));
local ImpErr ct_free
        OF ((int handle));
local ImpErr ct_loadf
        OF ((int handle, long *freq));
local ImpErr ct_ziprep
        OF ((int handle, U_CHAR **result));
local ImpErr ct_gencodes
        OF ((int handle, int minbits, int maxbits, long *saved));
local ImpErr ct_split
        OF ((TRDATA *part, int size, long freq,
             int prefix, int preflen, int minbits, int maxbits));
local int ct_fsort
        OF ((TRDATA *tr1, TRDATA *tr2));
local int ct_rsort
        OF ((RESORT *cr1, RESORT *cr2));
local
ImpErr
ct_alloc (size, handle)
    int size;
    int *handle;
{   register TRDATA *ct;
    int n;
#ifdef VALIDATE
    if (size < 2 || size > 256) goto badarg;
    if (handle == NULL)         goto badarg;
#endif /* VALIDATE */
    for (n = 0;
         n < MAXTREES && ct_table[n].ct_array != NULL;
         n++) ;
    if (n >= MAXTREES) return IM_NOCTBLS;
    *handle = n;
    ct_table[n].ct_size  = size;
    ct = (TRDATA *) malloc ((unsigned) (size * sizeof (TRDATA)));
    if (ct == NULL) return IM_NOMEM;
    ct_table[n].ct_array = ct;
    for (n = 0; n < size; n++, ct++)
    {   ct->ct_freq = 0;
        ct->ct_code = 0;
        ct->ct_val  = (U_CHAR)n;
        ct->ct_len  = 0;
    }
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_alloc: bad argument(s)");
    return IM_BADARG;
#endif /* VALIDATE */
}
local
ImpErr
ct_free (handle)
    int handle;
{
#ifdef VALIDATE
    if (!VALID_HANDLE (handle)) goto badarg;
#endif /* VALIDATE */
    free ((char *) ct_table[handle].ct_array);
    ct_table[handle].ct_array = NULL;
    ct_table[handle].ct_size  = 0;
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_free: bad argument(s)");
    return IM_BADARG;
#endif /* VALIDATE */
}
local
ImpErr
ct_loadf (handle, freq)
    int   handle;
    long *freq;
{   register long *f;
    register TRDATA *ct;
    int n;
#ifdef VALIDATE
    if (!VALID_HANDLE (handle)) goto badarg;
#endif /* VALIDATE */
    for (f = freq,
             ct = ct_table[handle].ct_array,
             n = ct_table[handle].ct_size;
         n > 0;
         f++, ct++, n--)
        ct->ct_freq += *f;
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_loadf: bad argument(s)");
    return IM_BADARG;
#endif /* VALIDATE */
}
#ifdef IMPDEBUG
char *treename;
#endif /* IMPDEBUG */
local
ImpErr
ct_ziprep (handle, result)
    int      handle;
    U_CHAR **result;
{   static U_CHAR buffer[257];          /* result info */
    register U_CHAR *c;
    register TRDATA *ct;
    int s, n, l;
#ifdef VALIDATE
    if (!VALID_HANDLE (handle)) goto badarg;
    if (result == NULL)         goto badarg;
#endif /* VALIDATE */
#ifdef  IMPDEBUG
    if (treename != NULL && treename[0] != 0)
    {   /* Print the code tree info. */
        fprintf (stderr, "\n%s tree:\n  value      len   string\n",
                 treename);
        for (ct = ct_table[handle].ct_array,
                  s = ct_table[handle].ct_size,
                  n = 0;
             s > 0;
             ct++, n++, s--)
            fprintf (stderr, "  %3d (%02x)    %2d    %04x (rev %04x)\n",
                     n, n, ct->ct_len,
                     bi_reverse(ct->ct_code << (16 - ct->ct_len),
                                ct->ct_len) << (16 - ct->ct_len),
                     ct->ct_code);
    }
#endif  /* IMPDEBUG */
    for (c = buffer+1,
             ct = ct_table[handle].ct_array,
             s = ct_table[handle].ct_size,
             n = 0,
             l = ct->ct_len;
         s > 0;
         ct++, s--)
    {   if (l < 1 || l > 16)
        {   fprintf (stderr, "\nError in ct_ziprep: bad code length");
            return IM_LOGICERR;
        }
        if (n >= 16 || (int)ct->ct_len != l)
        {   *c++ = (U_CHAR)((((n-1) << 4) & 0xf0) | ((l-1) & 0x0f));
            n = 1; l = ct->ct_len;
        }
        else n++;
    }
    if (n > 0)
        *c++ = (U_CHAR)((((n-1) << 4) & 0xf0) | ((l-1) & 0x0f));
    buffer[0] = (U_CHAR)((c - buffer) - 2);
    *result = buffer;
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_ziprep: bad argument(s)");
    return IM_BADARG;
#endif /* VALIDATE */
}
#define ct_lookup(handle, value, string, length) \
{   register TRDATA *ct; \
    ct = ct_table[handle].ct_array + (value); \
    string = ct->ct_code; \
    length = ct->ct_len; \
}
local
ImpErr
ct_gencodes (handle, minbits, maxbits, saved)
    int   handle;                       /* which tree */
    int   minbits;                      /* min code string bit length */
    int   maxbits;                      /* max code string bit length */
    long *saved;                        /* how many bits saved */
{   register TRDATA *ct;
    TRDATA *ct2;
    register int n;
    UL_INT f;
    register RESORT *cr;
    int code, srclen;
    long totalfreq, totalbits;
    ImpErr retcode;
    RESORT rbuf[256];
    int size; /* alias for ct_table[handle].ct_size */
    int z;    /* index of zero frequency element */
    int nz;   /* index of non zero frequency element */
#ifdef VALIDATE
    if (!VALID_HANDLE (handle)) goto badarg;
    if (minbits < 1)            goto badarg;
    if (maxbits > 16)           goto badarg;
    if (maxbits < minbits)      goto badarg;
    if (saved == NULL)          goto badarg;
#endif /* VALIDATE */
    size = ct_table[handle].ct_size;
    totalfreq = 0;
    ct = ct_table[handle].ct_array;
    ct2 = (TRDATA*) ma_buf;
    memcpy((char*)ct2, (char*)ct, size * sizeof(TRDATA));
    for (nz = 0, z = n = size-1; n >= 0; n--) {
        int m;
        if (ct2[n].ct_freq != 0L) {
            m = nz++;
            totalfreq += (ct[m].ct_freq = ct2[n].ct_freq);
        } else {
            m = z--;
            ct[m].ct_freq = 0L;
        }
        ct[m].ct_code = 0;
        ct[m].ct_len = 0;
        ct[m].ct_val = (U_CHAR)n; /* ct2[n].ct_val */
    }
    qsort ((char *) (ct_table[handle].ct_array), nz,
           sizeof (TRDATA), (int (*)())ct_fsort);
    retcode =
        ct_split (ct_table[handle].ct_array,    /* partition start */
                  size,                         /* partition size */
                  totalfreq,                    /* total frequency */
                  0,                            /* code string prefix */
                  0,                            /* # bits in prefix */
                  minbits,                      /* minimum tree depth */
                  maxbits);                     /* maximum tree depth */
    if (retcode != IM_OK) return retcode;
    n = size;
    if (n == 256)
    {   for (ct = ct_table[handle].ct_array;
             n > 0 && ct->ct_val != 255;
             n--, ct++) ;
        if (n == 0)
        {   fprintf (stderr, "\nError in ct_gencodes: no value 255");
            return IM_LOGICERR;
        }
        if (ct->ct_len < 10)
        {   ct2 = ct;
            while (n > 0 && ct->ct_len < 10) n--, ct++;
            if (n == 0) ct--;   /* no len>=10 in tree; use longest */
            n = ct->ct_val;
                ct->ct_val = ct2->ct_val;
                ct2->ct_val = (U_CHAR)n;
            f = ct->ct_freq;
                ct->ct_freq = ct2->ct_freq;
                ct2->ct_freq = f;
    }   }
    for (n = size,
             ct = ct_table[handle].ct_array,
             cr = rbuf;
         n > 0;
         n--, ct++, cr++)
    {   cr->ct_rlen = ct->ct_len;
        cr->ct_rval = ct->ct_val;
    }
    n = size;
    qsort ((char *) rbuf, n, sizeof (RESORT), (int (*)())ct_rsort);
    for (ct = ct_table[handle].ct_array,
             cr = rbuf;
         n > 0;
         n--, ct++, cr++)
        ct->ct_val = cr->ct_rval;
#ifdef DUMP_TREE
    printf ("Finished tree:\n");
    for (n = size,
             ct = ct_table[handle].ct_array;
         n > 0;
         n--, ct++)
        printf ("  %3d (0x%02x)  l %2d  c 0x%04x f %ld\n",
                ct->ct_val, ct->ct_val, ct->ct_len, ct->ct_code, ct->ct_freq);
    putchar ('\n');
#endif /* DUMP_TREE */
    ct = ct_table[handle].ct_array;
    ct2 = (TRDATA*)ma_buf;
    memcpy((char*)ct2, (char*)ct, size * sizeof(TRDATA));
    for (n = size-1; n >= 0; n--) {
        U_INT v = (U_INT) ct2[n].ct_val;
        ct[v] = ct2[n];
    }
    n = ct_table[handle].ct_size;
    for (code = 1, srclen = 0; code < n; code <<= 1, srclen++) ;
    for (ct = ct_table[handle].ct_array,
             totalbits = 0;
         n > 0;
         n--, ct++)
        totalbits += ct->ct_freq * ct->ct_len;
    *saved = (totalfreq * srclen) - totalbits;
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_gencodes: bad argument(s)");
    return IM_BADARG;
#endif /* VALIDATE */
}
local
ImpErr
ct_split (part, size, freq, prefix, preflen, minbits, maxbits)
    TRDATA *part;               /* start of partition */
    int     size;               /* # elements in partition */
    long    freq;               /* sum of frequencies in partition */
    int     prefix;             /* initial code bits for partition */
    int     preflen;            /* # bits in prefix */
    int     minbits;            /* minimum permissible bit length */
    int     maxbits;            /* maximum permissible bit length */
{   register TRDATA *ct;
    int topmaxbits, botmaxbits, localminbits;
    U_INT topmaxvals, botmaxvals;
    int topsize, botsize;
    long topfreq, botfreq, halffreq, onefreq;
    int n, m, leadzeros;
    int maxshort, minlong;
    ImpErr retcode;
    static maxarray[17] =
        { 8,8,8,8,12,12,14,14,16,16,16,16,16,16,16,16,16 };
#ifdef VALIDATE
    if (part == NULL)           goto badarg;
    if (size < 1)               goto badarg;
    if (freq < 0)               goto badarg;
    if (preflen < 0)            goto badarg;
    if (preflen > maxbits)      goto badarg;
    if (minbits < preflen)      goto badarg;
    if (maxbits > 16)           goto badarg;
    if (maxbits < minbits)      goto badarg;
#endif /* VALIDATE */
    if (size == 1)
    {   part->ct_code = bi_reverse(prefix, preflen);
        part->ct_len  = (U_CHAR)preflen;
        return IM_OK;
    }
    botmaxbits = maxbits;
    if (prefix != 0) topmaxbits = maxbits;
    else
    {   for (n = 0, leadzeros = 0x8000;
             n < preflen && (prefix & leadzeros) == 0;
             n++, leadzeros >>= 1) ;
        topmaxbits = maxarray[n];
        if (topmaxbits > maxbits) topmaxbits = maxbits;
    }
    if (topmaxbits < minbits)
    {   fprintf (stderr, "\nError in ct_split: ");
        fprintf (stderr, "topmaxbits(%d) < minbits(%d)",
                 topmaxbits, minbits);
        goto oops;
    }
    if (botmaxbits < minbits)
    {   fprintf (stderr, "\nError in ct_split: ");
        fprintf (stderr, "botmaxbits(%d) < minbits(%d)",
                 botmaxbits, minbits);
        goto oops;
    }
    topmaxvals = 1 << (topmaxbits - preflen - 1);
    n = size >> 1; if (topmaxvals > n) topmaxvals = n;
    botmaxvals = 1 << (botmaxbits - preflen - 1);
    n = size - 1;  if (botmaxvals > n) botmaxvals = n;
    if (topmaxvals + botmaxvals < size)
    {   fprintf (stderr, "\nError in ct_split: ");
        fprintf (stderr, "topmaxvals(%d) + botmaxvals(%d) ",
                 topmaxvals, botmaxvals);
        fprintf (stderr, "< size(%d)", size);
        goto oops;
    }
    if (freq == 0)
    {   topsize = size >> 1;
        ct = part + topsize;
        topfreq = 0;
    }
    else
    {   halffreq = freq >> 1;           /* half the total frequency */
        m = size >> 1;                  /* half the total elements, */
        for (topsize = 0, topfreq = 0, ct = part;
             topsize < m && topfreq <= halffreq
                 && (onefreq = ct->ct_freq) > 0;
             topsize++, ct++)
            topfreq += onefreq;
        if (topsize >= 2)
        {   
            onefreq = (ct-1)->ct_freq;
            if ((topfreq - halffreq) > (halffreq - (topfreq - onefreq)))
                ct--, topsize--, topfreq -= onefreq;
    }   }
    botsize = size - topsize;
    botfreq = freq - topfreq;
    while (topsize > topmaxvals)
    {   onefreq = (--ct)->ct_freq;
        topsize--; topfreq -= onefreq;
        botsize++; botfreq += onefreq;
    }
    while (botsize > botmaxvals)
    {   onefreq = (ct++)->ct_freq;
        topsize++; topfreq += onefreq;
        botsize--; botfreq -= onefreq;
    }
    localminbits = preflen + 1;
    if (localminbits < minbits) localminbits = minbits;
    for (;;)
    {   for (maxshort = preflen + 1, n = 1;
             n < botsize;
             maxshort++, n <<= 1) ;
        if (n > botsize) maxshort--;
        if (maxshort < localminbits) maxshort = localminbits;
        if (maxshort > topmaxbits) maxshort = topmaxbits;
        for (minlong = preflen + 1, n = 1;
             n < topsize;
             minlong++, n <<= 1) ;
        if (minlong <= maxshort) break;
        onefreq = (--ct)->ct_freq;
        topsize--; topfreq -= onefreq;
        botsize++; botfreq += onefreq;
    }
    n = 1 << (minbits - preflen - 1);
    while (topsize < n)
    {   onefreq = (ct++)->ct_freq;
        topsize++; topfreq += onefreq;
        botsize--; botfreq -= onefreq;
    }
    retcode = ct_split (part, topsize, topfreq,
                        prefix | (1 << (15-preflen)),
                        preflen + 1, localminbits, maxshort);
    if (retcode != IM_OK) return retcode;
    ct = part + topsize;
    retcode = ct_split (ct, botsize, botfreq,
                        prefix, preflen + 1, (int)ct[-1].ct_len, maxbits);
    if (retcode != IM_OK) return retcode;
    return IM_OK;
#ifdef VALIDATE
badarg:
    fprintf (stderr, "\nError in ct_split: bad argument(s)");
    putchar ('\n'); fflush (stdout); fflush (stderr);
    return IM_BADARG;
#endif /* VALIDATE */
oops:
#ifdef VALIDATE
    putchar ('\n'); fflush (stdout); fflush (stderr);
#endif /* VALIDATE */
    return IM_LOGICERR;
}
local
int
ct_fsort (tr1, tr2)
    TRDATA *tr1, *tr2;
{   long d;
    int v;
    d = (long) tr1->ct_freq - (long) tr2->ct_freq;
    if (d < 0) return 1;
    if (d > 0) return -1;
    v = (int) tr1->ct_val - (int) tr2->ct_val;
    if (v < 0) return 1;
    if (v > 0) return -1;
    return 0;
}
local
int
ct_rsort (cr1, cr2)
    RESORT *cr1, *cr2;
{   int d;
    d = (int) cr1->ct_rlen - (int) cr2->ct_rlen;
    if (d > 0) return 1;
    if (d < 0) return -1;
    d = (int) cr1->ct_rval - (int) cr2->ct_rval;
    if (d > 0) return 1;
    if (d < 0) return -1;
    return 0;
}
ImpErr
ct_init ()
{   ImpErr retcode;
    int i;
#ifdef DEBUG
    if (256*sizeof(TRDATA) > MA_BUFSIZE*sizeof(MATCH)) return IM_LOGICERR;
#endif /* DEBUG */
    retcode = ct_windup ();
        if (retcode != IM_OK) return retcode;
    ct_litc_num = 0;
    ct_lit2_num = 0;
    ct_strg_num = 0;
    for (i = 255; i >= 0;  i--)
        ct_litc_freq[i] = 0;
    for (i = 63; i >= 0; i--)
        ct_len2_freq[i] = 0, ct_len3_freq[i] = 0,
        ct_dst2_freq[i] = 0, ct_dst3_freq[i] = 0;
    retcode = ct_alloc (256, &ct_litc_tree);
    if (retcode != IM_OK) return retcode;
    retcode = ct_alloc  (64, &ct_len2_tree);
    if (retcode != IM_OK) return retcode;
    retcode = ct_alloc  (64, &ct_len3_tree);
    if (retcode != IM_OK) return retcode;
    retcode = ct_alloc  (64, &ct_dst2_tree);
    if (retcode != IM_OK) return retcode;
    retcode = ct_alloc  (64, &ct_dst3_tree);
    if (retcode != IM_OK) return retcode;
    return IM_OK;
}
ImpErr
ct_tally (ma)
         MATCH *ma;             /* match data to write out */
{   register int ch;
    int dist = ma->ma_dist;
    if (dist == 0) {                 /* literal character */
            ct_litc_num++;
            ch = ma->l.ma_litc[0];
                ct_litc_freq[ch]++;
    } else if (dist < 0) {           /* 2-character match */
            ct_lit2_num++;
            ch = ma->l.ma_litc[0];
                ct_litc_freq[ch]++;
            ch = ma->l.ma_litc[1];
                ct_litc_freq[ch]++;
            ch = ((-dist-1) >> fd.fd_nbits) & 0x3f;
                ct_dst2_freq[ch]++;
            ct_len2_freq[0]++;
     } else {                        /* 3-char or longer match */
            ct_strg_num++;
            ch = ((dist-1) >> fd.fd_nbits) & 0x3f;
                ct_dst3_freq[ch]++;
            ch = ma->l.ma_length - 3;
                if (ch > 63) ch = 63;
                ct_len3_freq[ch]++;
    }
    return IM_OK;
}
ImpErr
ct_mktrees ()
{   U_CHAR *c;
    ImpErr retcode;
    register long sum;
    long len2, len3;
    int n;
    for (n = 62; n >= 0; n--) {
        ct_dst2_freq[n] += ct_dst3_freq[n];
        ct_len2_freq[n+1] += ct_len3_freq[n];
    }
    ct_dst2_freq[63] += ct_dst3_freq[63];
    ct_len2_freq[63] += ct_len3_freq[63];
#ifdef IMPDEBUG
    treename = (char *)NULL;
#endif /* IMPDEBUG */
    retcode = ct_loadf    (ct_litc_tree,  ct_litc_freq);
        if (retcode != IM_OK) return retcode;
    retcode = ct_gencodes (ct_litc_tree, 1, 16, &ct_litc_saved);
        if (retcode != IM_OK) return retcode;
    retcode = ct_ziprep   (ct_litc_tree, &c);
        if (retcode != IM_OK) return retcode;
    ct_litc_saved -= (int) (c[0]+2) * 8;
    retcode = ct_loadf    (ct_len2_tree,  ct_len2_freq);
        if (retcode != IM_OK) return retcode;
    retcode = ct_gencodes (ct_len2_tree, 1, 16, &ct_len2_saved);
        if (retcode != IM_OK) return retcode;
    retcode = ct_ziprep   (ct_len2_tree, &c);
        if (retcode != IM_OK) return retcode;
    ct_len2_saved -= (int) (c[0]+2) * 8;
    retcode = ct_loadf    (ct_len3_tree,  ct_len3_freq);
        if (retcode != IM_OK) return retcode;
    retcode = ct_gencodes (ct_len3_tree, 1, 16, &ct_len3_saved);
        if (retcode != IM_OK) return retcode;
    retcode = ct_ziprep   (ct_len3_tree, &c);
        if (retcode != IM_OK) return retcode;
    ct_len3_saved -= (int) (c[0]+2) * 8;
    retcode = ct_loadf    (ct_dst2_tree,  ct_dst2_freq);
        if (retcode != IM_OK) return retcode;
    retcode = ct_gencodes (ct_dst2_tree, 1,  8, &ct_dst2_saved);
        if (retcode != IM_OK) return retcode;
    retcode = ct_ziprep   (ct_dst2_tree, &c);
        if (retcode != IM_OK) return retcode;
    ct_dst2_saved -= (int) (c[0]+2) * 8;
    retcode = ct_loadf    (ct_dst3_tree,  ct_dst3_freq);
        if (retcode != IM_OK) return retcode;
    retcode = ct_gencodes (ct_dst3_tree, 1,  8, &ct_dst3_saved);
        if (retcode != IM_OK) return retcode;
    retcode = ct_ziprep   (ct_dst3_tree, &c);
        if (retcode != IM_OK) return retcode;
    ct_dst3_saved -= (int) (c[0]+2) * 8;
    sum  = ct_litc_num + ct_lit2_num + ct_strg_num;    /* initial bit */
    sum += ct_litc_num * 8;                          /* literal bytes */
    sum += (ct_lit2_num+ct_strg_num) * 6 - ct_len2_saved;  /* lengths */
    sum += 8 * ct_len2_freq[63];                  /* oversize lengths */
    sum += (ct_lit2_num+ct_strg_num) * (fd.fd_nbits+6)
            - ct_dst2_saved;                             /* distances */
    len2 = (sum+7) / 8;                           /* convert to bytes */
    sum  = ct_litc_num + 2*ct_lit2_num + ct_strg_num;  /* initial bit */
    sum += (ct_litc_num+2*ct_lit2_num)*8 - ct_litc_saved;/* lit bytes */
    sum += ct_strg_num * 6 - ct_len3_saved;                /* lengths */
    sum += 8 * ct_len3_freq[63];                  /* oversize lengths */
    sum += ct_strg_num * (fd.fd_nbits+6) - ct_dst3_saved;   /* dist's */
    len3 = (sum+7) / 8;                           /* convert to bytes */
    if (ct_table[ct_litc_tree].ct_array[255].ct_len < 10)
        len3 = len2;
    if (len2 <= len3)
    {   fd.fd_method = NO_LITERAL_TREE;
        fd.fd_clen   = len2;
        lit_tree     = -1;
        len_tree     = ct_len2_tree;
        dst_tree     = ct_dst2_tree;
        retcode = ct_free (ct_litc_tree);
            if (retcode != IM_OK) return retcode;
        retcode = ct_free (ct_dst3_tree);
            if (retcode != IM_OK) return retcode;
        retcode = ct_free (ct_len3_tree);
            if (retcode != IM_OK) return retcode;
    }
    else
    {   fd.fd_method = LITERAL_TREE;
        fd.fd_clen   = len3;
        lit_tree     = ct_litc_tree;
        len_tree     = ct_len3_tree;
        dst_tree     = ct_dst3_tree;
        retcode = ct_free (ct_dst2_tree);
            if (retcode != IM_OK) return retcode;
        retcode = ct_free (ct_len2_tree);
            if (retcode != IM_OK) return retcode;
    }
    return IM_OK;
}
ImpErr
ct_wrtrees (outfp)
    FILE *outfp;                        /* output file */
{   ImpErr retcode;
    U_CHAR *c;
#ifdef IMPDEBUG
    treename = "Literal";
#endif /* IMPDEBUG */
    if (lit_tree >= 0)
    {   retcode = ct_ziprep (lit_tree, &c);
            if (retcode != IM_OK) return retcode;
        if (zfwrite ((char *) c, (int) (c[0]+2), 1, outfp) != 1)
            return IM_IOERR;
    }
#ifdef IMPDEBUG
    treename = "Length";
#endif /* IMPDEBUG */
    retcode = ct_ziprep (len_tree, &c);
        if (retcode != IM_OK) return retcode;
    if (zfwrite ((char *) c, (int) (c[0]+2), 1, outfp) != 1)
        return IM_IOERR;
#ifdef IMPDEBUG
    treename = "Distance";
#endif /* IMPDEBUG */
    retcode = ct_ziprep (dst_tree, &c);
        if (retcode != IM_OK) return retcode;
    if (zfwrite ((char *) c, (int) (c[0]+2), 1, outfp) != 1)
        return IM_IOERR;
    return IM_OK;
}
#define OUTBITS(value,length) \
        {   retcode = bi_rlout ((int) (value), (int) (length)); \
                if (retcode != IM_OK) return retcode; \
        }
#define OUTCODE(value,tree) \
        {   ct_lookup (tree, value, bitstring, bitlength); \
            retcode = bi_rlout (bitstring, bitlength); \
                if (retcode != IM_OK) return retcode; \
        }
ImpErr
ct_wrdata (outfp)
         FILE *outfp;                   /* output (ZIP) file */
{   MATCH *ma;
    ImpErr retcode;
    register int minmatch;
    int bitstring, bitlength;
    int bitmask = (1 << (fd.fd_nbits+1))-1;
    int matches;
#ifdef  IMPDEBUG
    long srcpos;
#endif  /* IMPDEBUG */
    minmatch = (lit_tree >= 0) ? 3 : 2;
    if (tflush (fd.fd_temp) != 0) return IM_IOERR;
    trewind (fd.fd_temp);
    retcode = bi_init (outfp);
        if (retcode != IM_OK) return retcode;
#ifdef  IMPDEBUG
        srcpos = 0;
        fprintf (stderr, "\nImploded output:\n");
#endif  /* IMPDEBUG */
    while ((matches =
       tread ((char *) ma_buf, sizeof(MATCH), MA_BUFSIZE, fd.fd_temp)) > 0)
       for (ma = ma_buf; matches > 0; ma++, matches--)
    {
        int dist = ma->ma_dist;
        int len = 0;
#ifdef  IMPDEBUG
        fprintf (stderr, "%8ld: ", srcpos);
#endif  /* IMPDEBUG */
        if (dist < 0) {
            dist = -dist, len = 2;
        } else if (dist > 0) {
            len = ma->l.ma_length;
        }
        if (len >= minmatch)
        {   /* "matched string" header bit (0) */
#ifdef  IMPDEBUG
            fprintf (stderr, "str (dst=%d,len=%d)  ",
                     dist, len);
            srcpos += len;
#endif  /* IMPDEBUG */
            dist--;
            OUTBITS ((dist << 1) & bitmask, fd.fd_nbits + 1);
            OUTCODE (dist >> fd.fd_nbits, dst_tree);
            len -= minmatch;
            if (len >= 63)
            {   /* big length -- output code for 63, then surplus */
                OUTCODE (63, len_tree);
                OUTBITS ((len - 63), 8);
            }
            else
            {   /* small length -- output code */
                OUTCODE (len, len_tree);
        }   }
        else if (lit_tree >= 0)
        {   /* first or single literal -- header bit (1) plus char */
#ifdef  IMPDEBUG
            fprintf (stderr, "lit (val=%02x)  ",
                     ma->l.ma_litc[0] & 0xff);
            srcpos++;
#endif  /* IMPDEBUG */
            OUTBITS (1, 1);
            OUTCODE (ma->l.ma_litc[0], lit_tree);
            if (len == 2)
            {   /* second literal -- header bit (1) plus char */
#ifdef  IMPDEBUG
                fprintf (stderr, "\n%8ld: lit (val=%02x)  ",
                         srcpos, ma->l.ma_litc[1] & 0xff);
                srcpos++;
#endif  /* IMPDEBUG */
                OUTBITS (1, 1);
                OUTCODE (ma->l.ma_litc[1], lit_tree);
        }   }
        else
        {   /* single literal -- header bit (1) plus char */
#ifdef  IMPDEBUG
            fprintf (stderr, "lit (val=%02x)  ",
                     ma->l.ma_litc[0] & 0xff);
            srcpos++;
#endif  /* IMPDEBUG */
            OUTBITS ((ma->l.ma_litc[0] << 1) + 1, 9);
        }
#ifdef  IMPDEBUG
        putc ('\n', stderr);
#endif  /* IMPDEBUG */
    }
    if (terror (fd.fd_temp)
#ifndef MINIX
#ifndef __TURBOC__      /* TurboC 2.0 does not set the EOF flag (?) */
        || !teof (fd.fd_temp)
#endif /* !__TURBOC__ */
#endif /* !MINIX */
       )
      return IM_IOERR;
    retcode = bi_windup ();
        if (retcode != IM_OK) return retcode;
    return IM_OK;
}
#undef  OUTBITS
#undef  OUTCODE
ImpErr
ct_windup ()
{   int n;
    static windup_already_called = 0;
    ImpErr retcode;
    if (windup_already_called)
    {   /* Discard any old code trees. */
        for (n = 0; n < MAXTREES; n++)
        {   if (ct_table[n].ct_array != NULL)
            {   retcode = ct_free (n);
                if (retcode != IM_OK) return retcode;
    }   }   }
    else
    {   /* Initialize the list of active code trees. */
        for (n = 0; n < MAXTREES; n++)
        {   ct_table[n].ct_array = NULL;
            ct_table[n].ct_size  = 0;
        }
        windup_already_called = 1;
    }
    return IM_OK;
}
