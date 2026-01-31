/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "zip.h"
#include "tempf.h"
#define MINBITS         9       /* Starting code size of 9 bits */
#define MAXBITS         13      /* Maximum code size of 13 bits */
#define TABLESIZE       8191    /* We'll need 4K entries in table */
#define SPECIAL         256     /* Special function code */
#define INCSIZE         1       /* Code for a jump in code size */
#define CLEARCODE       2       /* Code for code table has been cleared */
#define FIRSTENTRY      257     /* First available table entry */
typedef struct CodeRec {
  short Child;          /* Addr of 1st suffix for this prefix */
  short Sibling;        /* Addr of next suffix in chain */
  uch Suffix;           /* Suffix character */
} CodeRec;
typedef CodeRec CodeArray[TABLESIZE + 1];       /* Define the code table */
local CodeRec far *CodeTable;   /* Points to code table for LZW compression */
local int NextFree;     /* Next free table entry */
local int CodeSize;     /* Size of codes (in bits) currently being written */
local int MaxCode;      /* Largest code that can be written in CodeSize bits */
local int FirstCh;      /* Flag indicating the START of a shrink operation */
local tFILE *tempf = NULL;      /* Temporary file */
local ulg count;                /* Count of bytes written */
#ifdef PROTO
   local void PutCode(int);
   local int Build_Data_Structures(void);
   local void Destroy_Data_Structures(void);
   local void Initialize_Data_Structures(void);
   local void Clear_Table(void);
   local void Table_Add(int, int);
#endif /* PROTO */
#define PUT(c) {tputc(c, tempf); count++;}
local void PutCode(c)
int c;                  /* code to send */
{
  static int b = 0;     /* current bits waiting to go out */
  static int n = 0;     /* number of bits in b */
  static int x[] = {0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff,
                0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff};
  if (c == -1)
  {
    if (n)
    {
      if (n > 8)
      {
        PUT((char)b)
        PUT((char)((b >> 8) & x[n - 8]))
      }
      else
        PUT((char)(b & x[n]))
      b = n = 0;
    }
  }
  else
  {
    b |= (c & x[CodeSize]) << n;
    n += CodeSize;
    if (n >= 16)
    {
      PUT((char)b)
      PUT((char)(b >> 8))
      if (n == 16)
        b = n = 0;
      else
      {
        n -= 16;
        b = (c >> (CodeSize - n)) & x[n];
      }
    }
  }
}
local int Build_Data_Structures()
{
  return (CodeTable = (CodeRec far *)farmalloc(sizeof(CodeArray))) == NULL;
}
local void Destroy_Data_Structures()
{
  if (CodeTable != NULL)
  {
    farfree((voidp far *)CodeTable);
    CodeTable = NULL;
  }
}
local void Initialize_Data_Structures()
{
  int i;                /* counter for table entries */
  CodeRec far *t;       /* pointer to current table entry */
  for (i = 0, t = CodeTable; i <= 255; i++, t++)
  {
    t->Child = -1;
    t->Suffix = (uch)i;
  }
  NextFree = FIRSTENTRY;
  for (i = FIRSTENTRY, t = CodeTable + FIRSTENTRY; i < TABLESIZE; i++, t++)
    t->Child = i + 1;
  t->Child = -1;
}
local void Clear_Table()
{
  int n;                /* node counter */
  CodeRec far *p;       /* pointer to next node to look at */
  short far *q;         /* pointer to node child or sibling entry */
  p = CodeTable + TABLESIZE;
  n = TABLESIZE + 1 - FIRSTENTRY;
  do {
    if (p->Child == -1)
      p->Child = -2;
    p--;
  } while (--n);
  p = CodeTable;
  n = 256;
  do {
    q = &p->Child;
    while (*q != -1 && CodeTable[*q].Child == -2)
      *q = CodeTable[*q].Sibling;
    p++;
  } while (--n);
  p = CodeTable + FIRSTENTRY;
  n = TABLESIZE + 1 - FIRSTENTRY;
  do {
    if (p->Child != -2)
    {
      q = &p->Child;
      while (*q != -1 && CodeTable[*q].Child == -2)
        *q = CodeTable[*q].Sibling;
      q = &p->Sibling;
      while (*q != -1 && CodeTable[*q].Child == -2)
        *q = CodeTable[*q].Sibling;
    }
    p++;
  } while (--n);
  NextFree = -1;
  p = CodeTable + TABLESIZE;
  n = TABLESIZE + 1 - FIRSTENTRY;
  do {
    if (p->Child == -2)
    {
      p->Child = NextFree;
      NextFree = n + FIRSTENTRY - 1;
    }
    p--;
  } while (--n);
}
local void Table_Add(p, s)
int p;                  /* prefix to add to */
int s;                  /* suffix to add to it */
{
  int f;                /* next free node */
  if ((f = NextFree) != -1)
  {
    NextFree = CodeTable[f].Child;
    CodeTable[f].Child = -1;
    CodeTable[f].Sibling = -1;
    CodeTable[f].Suffix = (uch)s;
    if (CodeTable[p].Child == -1)
      CodeTable[p].Child = f;
    else
    {
      p = CodeTable[p].Child;
      while (CodeTable[p].Sibling != -1)
        p = CodeTable[p].Sibling;
      CodeTable[p].Sibling = f;
    }
  }
}
local int lastcode;
int shr_setup()
{
  if (Build_Data_Structures())
    return ZE_MEM;
  Initialize_Data_Structures();
  FirstCh = 1;
  lastcode = -1;
  if ((tempf = topen('S')) == NULL)
    return ZE_MEM;
  count = 0;
  return ZE_OK;
}
int shr_p1(b, n)
uch *b;                 /* buffer with bytes to shrink */
extent n;               /* number of bytes in buffer */
{
  int f;                /* result of Table_Lookup */
  int s;                /* byte to shrink */
  if (FirstCh && n)
  {                             /* If just getting started ... */
    CodeSize = MINBITS;         /*   Initialize code size to minimum */
    MaxCode = (1 << CodeSize) - 1;
    lastcode = *b++;  n--;      /*   get first character from input, */
    FirstCh = 0;                /*   and reset the first char flag. */
  }
  while (NextFree == -1 && n)
  {
    PutCode(lastcode);
    PutCode(SPECIAL);
    PutCode(CLEARCODE);
    Clear_Table();
    Table_Add(lastcode, s = *b++);  n--;
    lastcode = s;
  }
  while (n)
  {
    s = *b++;  n--;
    f = CodeTable[lastcode].Child;
    while (f != -1 && CodeTable[f].Suffix != (uch)s)
      f = CodeTable[f].Sibling;
    if (f != -1)
      lastcode = f;
    else
    {
      PutCode(lastcode);        /* Write current lastcode */
      Table_Add(lastcode, s);   /* Attempt to add to code table */
      lastcode = s;             /* Reset lastcode for new char */
      if (NextFree > MaxCode && CodeSize < MAXBITS)
      {
        PutCode(SPECIAL);
        PutCode(INCSIZE);
        CodeSize++;
        MaxCode = (1 << CodeSize) - 1;
      }
      while (NextFree == -1 && n)
      {
        PutCode(lastcode);
        PutCode(SPECIAL);
        PutCode(CLEARCODE);
        Clear_Table();
        Table_Add(lastcode, s = *b++);  n--;
        lastcode = s;
      }
    }
  }
  return ZE_OK;
}
int shr_size(s)
ulg *s;                 /* return value: size of shrunk data */
{
  PutCode(lastcode);            /* Write last prefix code */
  PutCode(-1);                  /* Tell putcode to flush remaining bits */
  Destroy_Data_Structures();
  *s = count;
  return tflush(tempf) || terror(tempf) ? ZE_TEMP : ZE_OK;
}
int shr_p2(f)
FILE *f;                /* file to write shrunk data to */
{
  char *b;              /* malloc'ed buffer for copying */
  extent k;             /* holds result of fread */
  if ((b = malloc(BSZ)) == NULL)
    return ZE_MEM;
  trewind(tempf);
  while ((k = tread(b, 1, BSZ, tempf)) > 0)
    if (zfwrite(b, 1, k, f) != k)
    {
      free((voidp *)b);
      return ZE_TEMP;
    }
  free((voidp *)b);
  if (terror(tempf))
    return ZE_TEMP;
  tclose(tempf);
  tempf = NULL;
  return ZE_OK;
}
int shr_clear()
{
  Destroy_Data_Structures();
  if (tempf != NULL)
  {
    tclose(tempf);
    tempf =  NULL;
  }
  return ZE_OK;
}
