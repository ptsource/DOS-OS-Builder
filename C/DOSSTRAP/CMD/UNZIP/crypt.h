/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#ifndef __crypt_h   /* don't include more than once */
#define __crypt_h
#ifdef CRYPT
#  undef CRYPT      /* dummy version */
#endif
#define RAND_HEAD_LEN  12    /* needed to compile funzip */
#define zencode
#define zdecode
#define zfwrite  fwrite
#define echoff(f)
#define echon()
#if (defined(AMIGA) && !defined(EPIPE))
#  define EPIPE 9999         /* (errno == EPIPE) always false */
#endif
#endif /* !__crypt_h */
