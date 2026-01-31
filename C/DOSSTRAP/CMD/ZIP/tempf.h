/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#if !defined(OS2) && (defined(M_I86CM) || defined(__COMPACT__))
#   define TMPSIZ  0x8000  /* memory portion of temporary files */
#else
#   define TMPSIZ  0xe000  /* memory portion of temporary files */
#endif
typedef struct {
  char far *b;          /* memory part of file */
  unsigned p;           /* current read/write pointer for memory part */
  unsigned m;           /* bytes in memory part */
  int c;                /* character to use in spill file name */
  FILE *f;              /* spill file pointer or NULL*/
  char *n;              /* spill file name if f not NULL */
} tFILE;
tFILE *topen OF((int));
int tnew OF((tFILE *));
unsigned twrite OF((char *, unsigned, unsigned, tFILE *));
int tflush OF((tFILE *));
void trewind OF((tFILE *));
unsigned tread OF((char *, unsigned, unsigned, tFILE *));
int terror OF((tFILE *));
int teof OF((tFILE *));
int tclose OF((tFILE *));
#define tputcm(c,t) ((t)->b[(t)->p++]=(c),(t)->m<(t)->p?((t)->m=(t)->p):0,c)
#define tputcf(c,t) ((t)->f==NULL?(tnew(t)?-1:putc(c,(t)->f)):putc(c,(t)->f))
#define tputc(c,t) ((t)->p<TMPSIZ?(int)tputcm(c,t):tputcf(c,t))
