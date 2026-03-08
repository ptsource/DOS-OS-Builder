/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
#define INIT_BITS  9
#define FIRST_ENT  257
#define CLEAR      256
#define OUTB(c) {\
    *outptr++=(uch)(c);\
    if (++outcnt==outbufsiz) {\
        flush(outbuf,outcnt,TRUE);\
        outcnt=0L;\
        outptr=outbuf;\
    }\
}
static void partial_clear __((void));
int codesize, maxcode, maxcodemax, free_ent;
int unshrink()   /* return PK-type error code */
{
    register int code;
    register int stackp;
    int finchar;
    int oldcode;
    int incode;
    unsigned int outbufsiz;
#ifndef SMALL_MEM
    if (pInfo->textmode && !outbuf2 &&
        (outbuf2 = (uch *)malloc(TRANSBUFSIZ)) == NULL)
        return PK_MEM2;
#endif
    outptr = outbuf;
    outcnt = 0L;
    if (pInfo->textmode)
        outbufsiz = RAWBUFSIZ;
    else
        outbufsiz = OUTBUFSIZ;
    codesize = INIT_BITS;
    maxcode = (1 << codesize) - 1;
    maxcodemax = HSIZE;         /* (1 << MAX_BITS) */
    free_ent = FIRST_ENT;
    code = maxcodemax;
    do {
        prefix_of[code] = -1;
    } while (--code > 255);
    for (code = 255; code >= 0; code--) {
        prefix_of[code] = 0;
        suffix_of[code] = (uch)code;
    }
    READBITS(codesize,oldcode)  /* ; */
    if (zipeof)
        return PK_COOL;
    finchar = oldcode;
    OUTB(finchar)
    stackp = HSIZE;
    while (!zipeof) {
        READBITS(codesize,code)  /* ; */
        if (zipeof) {
            if (outcnt > 0L)
                flush(outbuf, outcnt, TRUE);   /* flush last, partial buffer */
            return PK_COOL;
        }
        while (code == CLEAR) {
            READBITS(codesize,code)  /* ; */
            switch (code) {
                case 1:
                    codesize++;
                    if (codesize == MAX_BITS)
                        maxcode = maxcodemax;
                    else
                        maxcode = (1 << codesize) - 1;
                    break;
                case 2:
                    partial_clear();
                    break;
            }
            READBITS(codesize,code)  /* ; */
            if (zipeof) {
                if (outcnt > 0L)
                    flush(outbuf, outcnt, TRUE);   /* partial buffer */
                return PK_COOL;
            }
        }
        incode = code;
        if (prefix_of[code] == -1) {
            stack[--stackp] = (uch)finchar;
            code = oldcode;
        }
        while (code >= FIRST_ENT) {
            if (prefix_of[code] == -1) {
                stack[--stackp] = (uch)finchar;
                code = oldcode;
            } else {
                stack[--stackp] = suffix_of[code];
                code = prefix_of[code];
            }
        }
        finchar = suffix_of[code];
        stack[--stackp] = (uch)finchar;
        if ((HSIZE - stackp + outcnt) < outbufsiz) {
            memcpy(outptr, &stack[stackp], HSIZE - stackp);
            outptr += HSIZE - stackp;
            outcnt += HSIZE - stackp;
            stackp = HSIZE;
        }
        else
            while (stackp < HSIZE)
                OUTB(stack[stackp++])
        code = free_ent;
        if (code < maxcodemax) {
            prefix_of[code] = oldcode;
            suffix_of[code] = (uch)finchar;
            do
                code++;
            while ((code < maxcodemax) && (prefix_of[code] != -1));
            free_ent = code;
        }
        oldcode = incode;
    }
    if (outcnt > 0L)
        flush(outbuf, outcnt, TRUE);
    return PK_OK;
} /* end function unshrink() */
static void partial_clear()
{
    register int pr;
    register int cd;
    for (cd = FIRST_ENT; cd < free_ent; cd++)
        prefix_of[cd] |= 0x8000;
    for (cd = FIRST_ENT; cd < free_ent; cd++) {
        pr = prefix_of[cd] & 0x7fff;    /* reference to another node? */
        if (pr >= FIRST_ENT)    /* flag node as referenced */
            prefix_of[pr] &= 0x7fff;
    }
    for (cd = FIRST_ENT; cd < free_ent; cd++)
        if ((prefix_of[cd] & 0x8000) != 0)
            prefix_of[cd] = -1;
    cd = FIRST_ENT;
    while ((cd < maxcodemax) && (prefix_of[cd] != -1))
        cd++;
    free_ent = cd;
}
