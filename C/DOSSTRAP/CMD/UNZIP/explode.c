/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
#ifndef WSIZE
#  define WSIZE 0x8000  /* window size--must be a power of two, and */
#endif                  /* at least 8K for zip's implode method */
struct huft {
  uch e;                /* number of extra bits or operation */
  uch b;                /* number of bits in this code or subcode */
  union {
    ush n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};
extern unsigned hufts;
int huft_build OF((unsigned *, unsigned, unsigned, ush *, ush *,
                   struct huft **, int *));
int huft_free OF((struct huft *));
int get_tree OF((unsigned *, unsigned));
int explode_lit8 OF((struct huft *, struct huft *, struct huft *,
                     int, int, int));
int explode_lit4 OF((struct huft *, struct huft *, struct huft *,
                     int, int, int));
int explode_nolit8 OF((struct huft *, struct huft *, int, int));
int explode_nolit4 OF((struct huft *, struct huft *, int, int));
int explode OF((void));
ush cplen2[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
        35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};
ush cplen3[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
        19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
        53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66};
ush extra[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        8};
ush cpdist4[] = {1, 65, 129, 193, 257, 321, 385, 449, 513, 577, 641, 705,
        769, 833, 897, 961, 1025, 1089, 1153, 1217, 1281, 1345, 1409, 1473,
        1537, 1601, 1665, 1729, 1793, 1857, 1921, 1985, 2049, 2113, 2177,
        2241, 2305, 2369, 2433, 2497, 2561, 2625, 2689, 2753, 2817, 2881,
        2945, 3009, 3073, 3137, 3201, 3265, 3329, 3393, 3457, 3521, 3585,
        3649, 3713, 3777, 3841, 3905, 3969, 4033};
ush cpdist8[] = {1, 129, 257, 385, 513, 641, 769, 897, 1025, 1153, 1281,
        1409, 1537, 1665, 1793, 1921, 2049, 2177, 2305, 2433, 2561, 2689,
        2817, 2945, 3073, 3201, 3329, 3457, 3585, 3713, 3841, 3969, 4097,
        4225, 4353, 4481, 4609, 4737, 4865, 4993, 5121, 5249, 5377, 5505,
        5633, 5761, 5889, 6017, 6145, 6273, 6401, 6529, 6657, 6785, 6913,
        7041, 7169, 7297, 7425, 7553, 7681, 7809, 7937, 8065};
#define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}
int get_tree(l, n)
unsigned *l;            /* bit lengths */
unsigned n;             /* number expected */
{
  unsigned i;           /* bytes remaining in list */
  unsigned k;           /* lengths entered */
  unsigned j;           /* number of codes */
  unsigned b;           /* bit length for those codes */
  i = NEXTBYTE + 1;                     /* length/count pairs to read */
  k = 0;                                /* next code */
  do {
    b = ((j = NEXTBYTE) & 0xf) + 1;     /* bits in code (1..16) */
    j = ((j & 0xf0) >> 4) + 1;          /* codes with those bits (1..16) */
    if (k + j > n)
      return 4;                         /* don't overflow l[] */
    do {
      l[k++] = b;
    } while (--j);
  } while (--i);
  return k != n ? 4 : 0;                /* should have read n of them */
}
int explode_lit8(tb, tl, td, bb, bl, bd)
struct huft *tb, *tl, *td;      /* literal, length, and distance tables */
int bb, bl, bd;                 /* number of bits decoded by those */
{
  long s;               /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned mb, ml, md;  /* masks for bb, bl, and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  mb = mask_bits[bb];           /* precompute masks for speed */
  ml = mask_bits[bl];
  md = mask_bits[bd];
  s = ucsize;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--decode it */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS((unsigned)bb)    /* get coded literal */
      if ((e = (t = tb + ((~(unsigned)b) & mb))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      slide[w++] = (uch)t->v.n;
      if (w == WSIZE)
      {
        flush(slide, w, 0);
        w = u = 0;
      }
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(7)               /* get distance low bits */
      d = (unsigned)b & 0x7f;
      DUMPBITS(7)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memzero(slide + w, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(slide, w, 0);
          w = u = 0;
        }
      } while (n);
    }
  }
  flush(slide, w, 0);
  if (csize + (k >> 3))   /* should have read csize bytes, but sometimes */
  {                       /* read one too many:  k>>3 compensates */
    used_csize = lrec.csize - csize - (k >> 3);
    return 5;
  }
  return 0;
}
int explode_lit4(tb, tl, td, bb, bl, bd)
struct huft *tb, *tl, *td;      /* literal, length, and distance tables */
int bb, bl, bd;                 /* number of bits decoded by those */
{
  long s;               /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned mb, ml, md;  /* masks for bb, bl, and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  mb = mask_bits[bb];           /* precompute masks for speed */
  ml = mask_bits[bl];
  md = mask_bits[bd];
  s = ucsize;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--decode it */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS((unsigned)bb)    /* get coded literal */
      if ((e = (t = tb + ((~(unsigned)b) & mb))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      slide[w++] = (uch)t->v.n;
      if (w == WSIZE)
      {
        flush(slide, w, 0);
        w = u = 0;
      }
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(6)               /* get distance low bits */
      d = (unsigned)b & 0x3f;
      DUMPBITS(6)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memzero(slide + w, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(slide, w, 0);
          w = u = 0;
        }
      } while (n);
    }
  }
  flush(slide, w, 0);
  if (csize + (k >> 3))   /* should have read csize bytes, but sometimes */
  {                       /* read one too many:  k>>3 compensates */
    used_csize = lrec.csize - csize - (k >> 3);
    return 5;
  }
  return 0;
}
int explode_nolit8(tl, td, bl, bd)
struct huft *tl, *td;   /* length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
{
  long s;               /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  s = ucsize;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--get eight bits */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS(8)
      slide[w++] = (uch)b;
      if (w == WSIZE)
      {
        flush(slide, w, 0);
        w = u = 0;
      }
      DUMPBITS(8)
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(7)               /* get distance low bits */
      d = (unsigned)b & 0x7f;
      DUMPBITS(7)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memzero(slide + w, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(slide, w, 0);
          w = u = 0;
        }
      } while (n);
    }
  }
  flush(slide, w, 0);
  if (csize + (k >> 3))   /* should have read csize bytes, but sometimes */
  {                       /* read one too many:  k>>3 compensates */
    used_csize = lrec.csize - csize - (k >> 3);
    return 5;
  }
  return 0;
}
int explode_nolit4(tl, td, bl, bd)
struct huft *tl, *td;   /* length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
{
  long s;               /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  s = ucsize;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--get eight bits */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS(8)
      slide[w++] = (uch)b;
      if (w == WSIZE)
      {
        flush(slide, w, 0);
        w = u = 0;
      }
      DUMPBITS(8)
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(6)               /* get distance low bits */
      d = (unsigned)b & 0x3f;
      DUMPBITS(6)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memzero(slide + w, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(slide, w, 0);
          w = u = 0;
        }
      } while (n);
    }
  }
  flush(slide, w, 0);
  if (csize + (k >> 3))   /* should have read csize bytes, but sometimes */
  {                       /* read one too many:  k>>3 compensates */
    used_csize = lrec.csize - csize - (k >> 3);
    return 5;
  }
  return 0;
}
int explode()
{
  unsigned r;           /* return codes */
  struct huft *tb;      /* literal code table */
  struct huft *tl;      /* length code table */
  struct huft *td;      /* distance code table */
  int bb;               /* bits for tb */
  int bl;               /* bits for tl */
  int bd;               /* bits for td */
  static unsigned l[256]; /* bit lengths for codes */
  bl = 7;
  bd = csize > 200000L ? 8 : 7;
  hufts = 0;                    /* initialize huft's malloc'ed */
  if (lrec.general_purpose_bit_flag & 4)
  {
    bb = 9;                     /* base table size for literals */
    if ((r = get_tree(l, 256)) != 0)
      return (int)r;
    if ((r = huft_build(l, 256, 256, NULL, NULL, &tb, &bb)) != 0)
    {
      if (r == 1)
        huft_free(tb);
      return (int)r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return (int)r;
    if ((r = huft_build(l, 64, 0, cplen3, extra, &tl, &bl)) != 0)
    {
      if (r == 1)
        huft_free(tl);
      huft_free(tb);
      return (int)r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return (int)r;
    if (lrec.general_purpose_bit_flag & 2)      /* true if 8K */
    {
      if ((r = huft_build(l, 64, 0, cpdist8, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        huft_free(tb);
        return (int)r;
      }
      r = explode_lit8(tb, tl, td, bb, bl, bd);
    }
    else                                        /* else 4K */
    {
      if ((r = huft_build(l, 64, 0, cpdist4, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        huft_free(tb);
        return (int)r;
      }
      r = explode_lit4(tb, tl, td, bb, bl, bd);
    }
    huft_free(td);
    huft_free(tl);
    huft_free(tb);
  }
  else
  {
    if ((r = get_tree(l, 64)) != 0)
      return (int)r;
    if ((r = huft_build(l, 64, 0, cplen2, extra, &tl, &bl)) != 0)
    {
      if (r == 1)
        huft_free(tl);
      return (int)r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return (int)r;
    if (lrec.general_purpose_bit_flag & 2)      /* true if 8K */
    {
      if ((r = huft_build(l, 64, 0, cpdist8, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        return (int)r;
      }
      r = explode_nolit8(tl, td, bl, bd);
    }
    else                                        /* else 4K */
    {
      if ((r = huft_build(l, 64, 0, cpdist4, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        return (int)r;
      }
      r = explode_nolit4(tl, td, bl, bd);
    }
    huft_free(td);
    huft_free(tl);
  }
#ifdef DEBUG
  fprintf(stderr, "<%u > ", hufts);
#endif /* DEBUG */
  return (int)r;
}
#undef NEXTBYTE
#undef NEEDBITS
#undef DUMPBITS
