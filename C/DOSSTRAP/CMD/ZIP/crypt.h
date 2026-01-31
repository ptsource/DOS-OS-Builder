/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "tailor.h"
#ifdef EXPORT
#  define zfwrite fwrite
#  define zputc putc
#else /* !EXPORT */
   extern int zfwrite OF((voidp *, extent, extent, FILE *));
   extern int zfputc OF((int, FILE *));
   extern char *key;
#  define zputc(b,f) (key!=NULL?zfputc(b,f):putc(b,f))
#endif /* ?EXPORT */
char *tempname OF((int));
#ifdef NeXT
   extern void free(voidp *);
   extern voidp *qsort(voidp *, extent, extent, int (*)());
   extern extent strlen(char *);
   extern int unlink(char *);
#endif /* NeXT */
