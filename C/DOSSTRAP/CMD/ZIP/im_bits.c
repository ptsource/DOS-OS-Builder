/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "implode.h"
local   FILE *  bi_fp;
local unsigned short bi_buf;
#define Buf_size (8 * 2*sizeof(char))
local int bi_valid;                  /* number of valid bits in bi_buf */
#define PUTSHORT(w) \
{  (void) zputc ((char)((w) & 0xff), bi_fp); \
   (void) zputc ((char)((US_INT)(w) >> 8), bi_fp); \
   if (ferror (bi_fp)) return IM_IOERR; \
}
#define PUTBYTE(w) \
{  (void) zputc ((char)((w) & 0xff), bi_fp); \
   if (ferror (bi_fp)) return IM_IOERR; \
}
ImpErr
bi_init (fp)
    FILE *fp;
{   if (fp == NULL)
    {   fprintf (stderr, "\nError in bi_init: null file pointer");
        return IM_LOGICERR;
    }
    bi_fp   = fp;
    bi_buf = 0;
    bi_valid = 0;
    return IM_OK;
}
ImpErr
bi_rlout (value, length)
    int value;
    int length; /* must be <= 16 */
{
    if (bi_valid > Buf_size - length) {
        bi_buf |= (value << bi_valid);
        PUTSHORT(bi_buf);
        bi_buf = (unsigned short)value >> (Buf_size - bi_valid);
        bi_valid += length - Buf_size;
    } else {
        bi_buf |= value << bi_valid;
        bi_valid += length;
    }
#ifdef  IMPDEBUG
    fprintf (stderr, " / ");
    while (length-- > 0)
    {
        putc ((value & 1) ? '1' : '0', stderr);
        value >>= 1;
    }
#endif  /* IMPDEBUG */
    return IM_OK;
}
int
bi_reverse (value, length)
    int value;
    int length;
{
    int result = 0;
    unsigned short lbit = 0x8000;
    unsigned short rbit = 1;
    while (length-- > 0) {
       if (value & lbit) result |= rbit;
       lbit >>= 1, rbit <<= 1;
    }
    return result;
}
ImpErr
bi_windup ()
{
    if (bi_valid > 8) {
        PUTSHORT(bi_buf);
    } else if (bi_valid > 0) {
        PUTBYTE(bi_buf);
    }
    bi_buf = 0;
    bi_valid = 0;
    return IM_OK;
}
