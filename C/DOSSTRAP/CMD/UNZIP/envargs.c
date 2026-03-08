/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
static int count_args __((char *));
static void mem_err __((void));
#if (defined(SCCS) && !defined(lint))  /* causes warnings:  annoying */
   static char *SCCSid = "@(#)envargs.c";
#endif
void envargs(Pargc, Pargv, envstr)
    int *Pargc;
    char ***Pargv, *envstr;
{
    char *getenv();
    char *envptr;       /* value returned by getenv */
    char *bufptr;       /* copy of env info */
    int argc = 0;       /* internal arg count */
    char ch;            /* spare temp value */
    char **argv;        /* internal arg vector */
    char **argvect;     /* copy of vector address */
    envptr = getenv(envstr);
    if (envptr == (char *)NULL || *envptr == 0)
        return;
    argc = count_args(envptr);
    bufptr = (char *)malloc(1+strlen(envptr));
    if (bufptr == (char *)NULL)
        mem_err();
    strcpy(bufptr, envptr);
    argv = (char **)malloc((argc+*Pargc+1)*sizeof(char *));
    if (argv == (char **)NULL)
        mem_err();
    argvect = argv;
    *(argv++) = *((*Pargv)++);
    do {
        *(argv++) = bufptr;
        while (((ch = *bufptr) != '\0') && ch != ' ')
            ++bufptr;
        if (ch == ' ')
            *(bufptr++) = '\0';
        while (((ch = *bufptr) != '\0') && ch == ' ')
            ++bufptr;
    } while (ch);
    argc += *Pargc;
    while (--(*Pargc))
        *(argv++) = *((*Pargv)++);
    *argv = (char *)NULL;
    *Pargv = argvect;
    *Pargc = argc;
}
static int count_args(s)
    char *s;
{
    int count = 0;
    char ch;
    do {
        ++count;
        while (((ch = *s) != '\0') && ch != ' ')
            ++s;
        while (((ch = *s) != '\0') && ch == ' ')
            ++s;
    } while (ch);
    return count;
}
static void mem_err()
{
    perror("Can't get memory for arguments");
    exit(2);
}
#ifdef TEST
main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    printf("Orig argv: %p\n", argv);
    dump_args(argc, argv);
    envargs(&argc, &argv, "ENVTEST");
    printf(" New argv: %p\n", argv);
    dump_args(argc, argv);
}
dump_args(argc, argv)
    int argc;
    char *argv[];
{
    int i;
    printf("\nDump %d args:\n", argc);
    for (i = 0; i < argc; ++i)
        printf("%3d %s\n", i, argv[i]);
}
#endif /* TEST */
#ifdef MSDOS   /* DOS_OS2?  DOS_NT_OS2? */
void mksargs(argcp, argvp)
    int *argcp;
    char ***argvp;
{
#ifndef MSC /* declared differently in MSC 7.0 headers, at least */
    extern char **environ;          /* environment */
#endif
    char        **envp;             /* pointer into environment */
    char        **newargv;          /* new argument list */
    char        **argp;             /* pointer into new arg list */
    int         newargc;            /* new argument count */
    if (environ == NULL || argcp == NULL || argvp == NULL || *argvp == NULL)
        return;
    for (envp = environ, newargc = 0; *envp != NULL && (*envp)[0] == '~';
         envp++, newargc++)
        ;
    if (newargc == 0)
        return;     /* no environment arguments */
    newargv = (char **) malloc(sizeof(char **) * (newargc+1));
    if (newargv == NULL)
        return;     /* malloc failed */
    for (argp = newargv, envp = environ; *envp != NULL && (*envp)[0] == '~';
         *argp++ = &(*envp++)[1])
        ;
    *argp = NULL;   /* null-terminate the list */
    *argcp = newargc;
    *argvp = newargv;
}
#endif /* MSDOS */
