/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
#if 0  /* this is not useful until it matches Amiga names insensitively */
#ifdef AMIGA        /* some other platforms might also want to use this */
#  define ANSI_CHARSET       /* MOVE INTO UNZIP.H EVENTUALLY */
#endif
#endif /* 0 */
#ifdef ANSI_CHARSET
#  ifdef ToLower
#    undef ToLower
#  endif
#  define IsUpper(c) (c>=0xC0 ? c<=0xDE && c!=0xD7 : c>=0x41 && c<=0x5A)
#  define ToLower(c) (IsUpper((uch) c) ? (unsigned) c | 0x20 : (unsigned) c)
#endif
#define Case(x)  (ic? ToLower(x) : (x))
#if 0                /* GRR:  add this to unzip.h someday... */
#if !(defined(MSDOS) && defined(DOSWILD))
#define match(s,p,ic)   (recmatch((uch *)p,(uch *)s,ic) == 1)
int recmatch OF((uch *pattern, uch *string, int ignore_case));
#endif
#endif /* 0 */
static int recmatch OF((uch *pattern, uch *string, int ignore_case));
int match(string, pattern, ignore_case)
    char *string, *pattern;
    int ignore_case;
{
#if (defined(MSDOS) && defined(DOSWILD))
    char *dospattern;
    int j = strlen(pattern);
    if ((dospattern = (char *)malloc(j+1)) != NULL) {
        strcpy(dospattern, pattern);
        if (!strcmp(dospattern+j-3, "*.*")) {
            dospattern[j-2] = '\0';                    /* nuke the ".*" */
        } else if (!strcmp(dospattern+j-2, "*.")) {
            char *p = strchr(string, '.');
            if (p) {   /* found a dot:  match fails */
                free(dospattern);
                return 0;
            }
            dospattern[j-1] = '\0';                    /* nuke the end "." */
        }
        j = recmatch((uch *)dospattern, (uch *)string, ignore_case);
        free(dospattern);
        return j == 1;
    } else
#endif /* MSDOS && DOSWILD */
    return recmatch((uch *)pattern, (uch *)string, ignore_case) == 1;
}
static int recmatch(p, s, ic)
    uch *p;               /* sh pattern to match */
    uch *s;               /* string to which to match it */
    int ic;               /* true for case insensitivity */
{
    unsigned int c;       /* pattern char or start of range in [-] loop */
    c = *p++;
    if (c == 0)
        return *s == 0;
#ifdef VMS
    if (c == '%')         /* GRR:  make this conditional, too? */
#else /* !VMS */
    if (c == '?')
#endif /* ?VMS */
        return *s ? recmatch(p, s + 1, ic) : 0;
#ifdef AMIGA
    if (c == '#' && *p == '?')     /* "#?" is Amiga-ese for "*" */
        c = '*', p++;
#endif /* AMIGA */
    if (c == '*') {
        if (*p == 0)
            return 1;
        for (; *s; s++)
            if ((c = recmatch(p, s, ic)) != 0)
                return (int)c;
        return 2;       /* 2 means give up--match will return false */
    }
    if (c == '[') {
        int e;          /* flag true if next char to be taken literally */
        uch *q;         /* pointer to end of [-] group */
        int r;          /* flag true to match anything but the range */
        if (*s == 0)                           /* need a character to match */
            return 0;
        p += (r = (*p == '!' || *p == '^'));   /* see if reverse */
        for (q = p, e = 0; *q; q++)            /* find closing bracket */
            if (e)
                e = 0;
            else
                if (*q == '\\')      /* GRR:  change to ^ for MS-DOS, OS/2? */
                    e = 1;
                else if (*q == ']')
                    break;
        if (*q != ']')               /* nothing matches if bad syntax */
            return 0;
        for (c = 0, e = *p == '-'; p < q; p++) {  /* go through the list */
            if (e == 0 && *p == '\\')             /* set escape flag if \ */
                e = 1;
            else if (e == 0 && *p == '-')         /* set start of range if - */
                c = *(p-1);
            else {
                unsigned int cc = Case(*s);
                if (*(p+1) != '-')
                    for (c = c ? c : *p; c <= *p; c++)  /* compare range */
                        if (Case(c) == cc)
                            return r ? 0 : recmatch(q + 1, s + 1, ic);
                c = e = 0;   /* clear range, escape flags */
            }
        }
        return r ? recmatch(q + 1, s + 1, ic) : 0;  /* bracket match failed */
    }
    if (c == '\\' && (c = *p++) == 0)     /* if \ at end, then syntax error */
        return 0;
    return Case((uch)c) == Case(*s) ? recmatch(p, ++s, ic) : 0;
} /* end function recmatch() */
#ifdef WILD_STAT_BUG   /* Turbo/Borland C, Watcom C, VAX C, Atari MiNT libs */
int iswild(p)
    char *p;
{
    for (; *p; ++p)
        if (*p == '\\' && *(p+1))
            ++p;
#ifdef VMS
        else if (*p == '%' || *p == '*')
#else /* !VMS */
#ifdef AMIGA
        else if (*p == '?' || *p == '*' || (*p=='#' && p[1]=='?') || *p == '[')
#else /* !AMIGA */
        else if (*p == '?' || *p == '*' || *p == '[')
#endif /* ?AMIGA */
#endif /* ?VMS */
            return TRUE;
    return FALSE;
} /* end function iswild() */
#endif /* WILD_STAT_BUG */
#ifdef TEST_MATCH
#define put(s) { fputs(s, stdout); fflush(stdout); }
void main(void)
{
    char pat[256], str[256];
    for (;;) {
        put("Pattern (return to exit): ");
        gets(pat);
        if (!pat[0])
            break;
        for (;;) {
            put("String (return for new pattern): ");
            gets(str);
            if (!str[0])
                break;
            printf("Case sensitive: %s  insensitive: %s\n",
              match(str, pat, 0) ? "YES" : "NO",
              match(str, pat, 1) ? "YES" : "NO");
        }
    }
    exit(0);
}
#endif /* TEST_MATCH */
