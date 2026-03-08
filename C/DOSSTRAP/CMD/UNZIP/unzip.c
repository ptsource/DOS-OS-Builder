/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"               /* includes, typedefs, macros, etc. */
#ifdef MSWIN
#  include "wizunzip.h"
#endif
#define RELEASE
#ifdef RELEASE
#  define UZ_VERSION  "5.1"   /* official release version */
#  define ZI_VERSION  "2.0"
#else
#  define UZ_VERSION  "5.1"   /* internal beta level */
#  define ZI_VERSION  "2.0"
#endif
#ifdef MACOS
   extern char *unzip_version = UZ_VERSION;
   extern char *zipinfo_version = ZI_VERSION;
#endif
int zipinfo_mode;     /* behave like ZipInfo or like normal UnZip? */
int aflag=0;          /* -a: do ASCII-EBCDIC and/or end-of-line translation */
int cflag=0;          /* -c: output to stdout */
int dflag=0;          /* -d: all args are files/dirs to be extracted */
int fflag=0;          /* -f: "freshen" (extract only newer files) */
int hflag=0;          /* -h: header line (zipinfo) */
int jflag=0;          /* -j: junk pathnames (unzip) */
int lflag=(-1);       /* -12slmv: listing format (zipinfo) */
int overwrite_none=0; /* -n: never overwrite files (no prompting) */
int overwrite_all=0;  /* -o: OK to overwrite files without prompting */
int force_flag=0;     /* (shares -o for now): force to override errors, etc. */
int qflag=0;          /* -q: produce a lot less output */
#ifdef DOS_NT_OS2
   int sflag=0;       /* -s: convert filename spaces (blanks) to underscores */
   int volflag=0;     /* -$: extract volume labels */
#endif
int tflag=0;          /* -t: test (unzip) or totals line (zipinfo) */
int uflag=0;          /* -u: "update" (extract only newer & brand-new files) */
int U_flag=0;         /* -U: preserve uppercase characters in filenames */
int vflag=0;          /* -v: (verbosely) list directory */
int V_flag=0;         /* -V: don't strip VMS version numbers */
#ifdef VMS
   int secinf=0;      /* -X: keep owner/protection */
#endif
int zflag=0;          /* -z: display the zipfile comment (only, for unzip) */
int filespecs;        /* number of real file specifications to be matched */
int xfilespecs;       /* number of excluded filespecs to be matched */
int process_all_files = 0;
int create_dirs;      /* used by main(), mapname(), checkdir() */
int extract_flag;
LONGINT real_ecrec_offset, expect_ecrec_offset;
long csize;           /* used by list_files(), ReadByte(): must be signed */
long ucsize;          /* used by list_files(), unReduce(), explode() */
long used_csize;      /* used by extract_or_test_member(), explode() */
static char *fnames[2] = {"*", NULL};   /* default filenames vector */
char **pfnames = fnames, **pxnames = &fnames[1];
char near sig[5];
char near answerbuf[10];
min_info info[DIR_BLKSIZ], *pInfo=info;
union work area;              /* see unzip.h for the definition of work */
ulg crc32val;
ush near mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
uch *inbuf, *inptr;             /* input buffer (any size is OK) and pointer */
int incnt;
ulg bitbuf;
int bits_left;
boolean zipeof;
char *wildzipfn, *zipfn;    /* GRR:  MSWIN:  must nuke any malloc'd zipfn... */
int zipfd;                      /* zipfile file handle */
LONGINT ziplen;
uch *hold;
char near local_hdr_sig[5];     /* initialize signatures at runtime so unzip */
char near central_hdr_sig[5];   /*  executable won't look like a zipfile */
char near end_central_sig[5];
cdir_file_hdr crec;             /* used in unzip.c, extract.c, misc.c */
local_file_hdr lrec;            /* used in unzip.c, extract.c */
ecdir_rec ecrec;                /* used in unzip.c, extract.c */
struct stat statbuf;            /* used by main, mapname, check_for_newer */
LONGINT cur_zipfile_bufstart;   /* extract_or_test_files, readbuf, ReadByte */
LONGINT extra_bytes = 0;        /* used in unzip.c, misc.c */
uch *extra_field = (uch *)NULL; /* used by VMS, Mac and OS/2 versions */
#ifdef MACOS
   short  gnVRefNum;
   long  glDirID;
   OSType  gostCreator;
   OSType  gostType;
   boolean  fMacZipped;
   boolean  macflag;
   CursHandle  rghCursor[4];    /* status cursors */
   short  giCursor = 0;
#endif
int mem_mode = 0;
int disk_full;
#ifdef SYMLINKS
   int symlnk;
#endif
FILE *outfile;
uch *outbuf;
uch *outptr;
ulg outcnt;                     /* number of chars stored in outbuf */
#ifdef SMALL_MEM
   uch *outbuf2;                /* initialized in main() (never changes) */
#else
   uch *outbuf2 = NULL;         /* malloc'd ONLY if unshrink and -a */
#endif
#ifdef MSWIN
   char *filename;
#else
   char near filename[FILNAMSIZ];
#endif
char *EndSigMsg = "\nnote:\
  didn't find end-of-central-dir signature at end of central dir.\n";
char *CentSigMsg =
  "error:  expected central file header signature not found (file #%u).\n";
char *SeekMsg =
  "error [%s]:  attempt to seek before beginning of zipfile\n%s";
#ifdef VMS
  char *ReportMsg = "\
  (please check that you have transferred or created the zipfile in the\n\
  appropriate BINARY mode--this includes ftp, Kermit, AND unzip'd zipfiles)\n";
#else
  char *ReportMsg = "\
  (please check that you have transferred or created the zipfile in the\n\
  appropriate BINARY mode and that you have compiled unzip properly)\n";
#endif
#ifdef MSWIN
#  include "winsetup.c"   /* duplicates some code in main() */
#else /* !MSWIN */
#ifdef THINK_C
 int unzip(argc, argv)
#else
 int main(argc, argv)   /* return PK-type error code (except under VMS) */
#endif
    int argc;
    char *argv[];
{
    char *p;
    int error=FALSE;
#if defined(__IBMC__) && defined(__DEBUG_ALLOC__)
    extern void DebugMalloc(void);
    atexit(DebugMalloc);
#endif
#ifdef MACOS
    int a;
    for (a = 0;  a < 4;  ++a)
        rghCursor[a] = GetCursor(a+128);
    giCursor = 0;
    aflag=cflag=dflag=fflag=jflag=qflag=tflag=U_flag=uflag=vflag=zflag = 0;
    local_hdr_sig[1] = central_hdr_sig[1] = end_central_sig[1] = '\0';
    error = FALSE;
    overwrite_none = overwrite_all = force_flag = 0;
#endif /* MACOS */
#ifdef MALLOC_WORK
    area.Slide = (uch *)calloc(8193, sizeof(short)+sizeof(char)+sizeof(char));
    area.shrink.Prefix_of = (short *)area.Slide;
    area.shrink.Suffix_of = area.Slide + (sizeof(short)*(HSIZE+1));
    area.shrink.Stack = area.Slide + (sizeof(short) + sizeof(char))*(HSIZE+1);
#endif
#ifdef __human68k__
    InitTwentyOne();
#endif
    signal(SIGINT, handler);
#ifdef SIGTERM                 /* some systems really have no SIGTERM */
    signal(SIGTERM, handler);
#endif
#ifdef SIGBUS
    signal(SIGBUS, handler);
#endif
#ifdef SIGSEGV
    signal(SIGSEGV, handler);
#endif
#ifdef DEBUG_STRUC
    printf("local_file_hdr size: %X\n",
           sizeof(local_file_hdr));
    printf("local_byte_hdr size: %X\n",
           sizeof(local_byte_hdr));
    printf("actual size of local headers: %X\n", LREC_SIZE);
    printf("central directory header size: %X\n",
           sizeof(cdir_file_hdr));
    printf("central directory byte header size: %X\n",
           sizeof(cdir_byte_hdr));
    printf("actual size of central dir headers: %X\n", CREC_SIZE);
    printf("end central dir record size: %X\n",
           sizeof(ecdir_rec));
    printf("end central dir byte record size: %X\n",
           sizeof(ec_byte_rec));
    printf("actual size of end-central-dir record: %X\n", ECREC_SIZE);
#endif /* DEBUG_STRUC */
#ifdef SFX
    zipfn = argv[0];
#endif
#ifndef NO_ZIPINFO
    if ((p = strrchr(argv[0], DIR_END)) == NULL)
        p = argv[0];
    else
        ++p;
    if (STRNICMP(p, "zipinfo", 7) == 0 || STRNICMP(p, "ii", 2) == 0 ||
        (argc > 1 && strncmp(argv[1], "-Z", 2) == 0))
    {
        zipinfo_mode = TRUE;
        envargs(&argc, &argv, ENV_ZIPINFO);
        error = zi_opts(&argc, &argv);
    } else
#endif
    {
        zipinfo_mode = FALSE;
        envargs(&argc, &argv, ENV_UNZIP);
        error = uz_opts(&argc, &argv);
    }
#ifdef MSDOS
    mksargs(&argc, &argv);
#endif
#ifdef SFX
    if (error)
#else
    if ((argc < 0) || error)
#endif
        RETURN(error);
#ifdef DOS_NT_OS2
    pfnames = argv;
    while (*pfnames != NULL) {
        char *q;
        for (q = *pfnames;  *q;  ++q)
            if (*q == '\\')
                *q = '/';
        ++pfnames;
    }
#endif
    wildzipfn = *argv++;
#ifdef OLD_EXDIR
    if (argc > 0) {
        if (extract_flag && !dflag) {
            create_dirs = !fflag;
            if ((error = checkdir(*argv, ROOT)) > 2)  /* mem, or file in way */
                RETURN(error);
            else if (!error) {   /* it IS extract-to dir, so adjust pointers */
                ++argv;
                --argc;
            }
        }
    }
    filespecs = argc;
    xfilespecs = 0;
    if (argc > 0) {
        char **pp = argv-1;
        pfnames = argv;
        while (*++pp)
            if (strcmp(*pp, "-x") == 0) {
                if (pp > argv) {
                    *pp = 0;           /* terminate pfnames */
                    filespecs = pp - pfnames;
                } else {
                    pfnames = fnames;  /* defaults */
                    filespecs = 0;
                }
                pxnames = pp + 1;      /* excluded-names ptr starts after -x */
                xfilespecs = argc - filespecs - 1;
                break;                 /* skip rest of args */
            }
        process_all_files = FALSE;
    } else
        process_all_files = TRUE;       /* for speed */
#else /* !OLD_EXDIR */
    filespecs = argc;
    xfilespecs = 0;
    if (argc > 0) {
        int in_files=FALSE, in_xfiles=FALSE;
        char **pp = argv-1;
        process_all_files = FALSE;
        pfnames = argv;
        while (*++pp) {
            Trace((stderr, "pp - argv = %d\n", pp-argv));
            if (!dflag && strcmp(*pp, "-d") == 0) {
                dflag = TRUE;
                if (in_files) {      /* ... zipfile ... -d exdir ... */
                    *pp = 0;                    /* terminate pfnames */
                    filespecs = pp - pfnames;
                    in_files = FALSE;
                } else if (in_xfiles) {
                    *pp = 0;                    /* terminate pxnames */
                    xfilespecs = pp - pxnames;
                }
                if (*++pp) {
                    if (extract_flag) {
                        create_dirs = !fflag;
                        if ((error = checkdir(*pp, ROOT)) > 2)
                            RETURN(error);   /* out of memory, or file in way */
                    } else
                        fprintf(stderr,
                          "caution:  not extracting; -d ignored\n");
                } else {
                    fprintf(stderr, "error:  must specify directory to which to\
 extract with -d option\n");
                    break;   /* no arguments left */
                }
                if (pp == argv+1)   /* ... zipfile -d exdir ... */
                    if (pp[1]) {
                        pfnames = pp + 1;  /* argv+2 */
                        filespecs = argc - (pfnames-argv);  /* for now... */
                    } else {
                        process_all_files = TRUE;
                        pfnames = fnames;  /* GRR: necessary? */
                        filespecs = 0;     /* GRR: necessary? */
                        break;
                    }
            } else if (!in_xfiles) {
                if (strcmp(*pp, "-x") == 0) {
                    in_xfiles = TRUE;
                    if (pp == argv || (pp == argv+2 && dflag)) {
                        pfnames = fnames;  /* defaults */
                        filespecs = 0;
                    } else if (in_files) {
                        *pp = 0;                   /* terminate pfnames */
                        filespecs = pp - pfnames;  /* adjust count */
                        in_files = FALSE;
                    }
                    pxnames = pp + 1;  /* excluded-names ptr starts after -x */
                    xfilespecs = argc - (pxnames-argv);  /* anything left... */
                } else
                    in_files = TRUE;
            }
        }
    } else
        process_all_files = TRUE;      /* for speed */
#endif /* ?OLD_EXDIR */
    inbuf = (uch *)malloc(INBUFSIZ + 4);    /* 4 extra for hold[] (below) */
    outbuf = (uch *)malloc(OUTBUFSIZ + 1);  /* 1 extra for string termin. */
    if ((inbuf == (uch *)NULL) || (outbuf == (uch *)NULL)) {
        fprintf(stderr, "error:  can't allocate unzip buffers\n");
        RETURN(PK_MEM);
    }
    hold = inbuf + INBUFSIZ;     /* to check for boundary-spanning signatures */
#ifdef SMALL_MEM
    outbuf2 = outbuf+RAWBUFSIZ;  /* never changes */
#endif
    RETURN(process_zipfiles());  /* keep passing errors back... */
} /* end main() */
int uz_opts(pargc, pargv)
    int *pargc;
    char ***pargv;
{
    char **argv, *s;
    int argc, c, error=FALSE, negative=0;
    argc = *pargc;
    argv = *pargv;
    while (--argc > 0 && (*++argv)[0] == '-') {
        s = argv[0] + 1;
        while ((c = *s++) != 0) {    /* "!= 0":  prevent Turbo C warning */
            switch (c) {
                case ('-'):
                    ++negative;
                    break;
                case ('a'):
                    if (negative) {
                        aflag = MAX(aflag-negative,0);
                        negative = 0;
                    } else
                        ++aflag;
                    break;
                case ('b'):
                    if (negative)
                        negative = 0;   /* do nothing:  "-b" is default */
                    else
                        aflag = 0;
                    break;
                case ('c'):
                    if (negative) {
                        cflag = FALSE, negative = 0;
#ifdef NATIVE
                        aflag = 0;
#endif
                    } else {
                        cflag = TRUE;
#ifdef NATIVE
                        aflag = 2;  /* so you can read it on the screen */
#endif
                    }
                    break;
                case ('d'):  /* arg after zipfn is stored dir, not extract-to */
#ifdef OLD_EXDIR
                    if (negative)
                        dflag = FALSE, negative = 0;
                    else
                        dflag = TRUE;
#endif
                    break;
                case ('e'):    /* just ignore -e, -x options (extract) */
                    break;
                case ('f'):    /* "freshen" (extract only newer files) */
                    if (negative)
                        fflag = uflag = FALSE, negative = 0;
                    else
                        fflag = uflag = TRUE;
                    break;
                case ('j'):    /* junk pathnames/directory structure */
                    if (negative)
                        jflag = FALSE, negative = 0;
                    else
                        jflag = TRUE;
                    break;
                case ('l'):
                    if (negative) {
                        vflag = MAX(vflag-negative,0);
                        negative = 0;
                    } else
                        ++vflag;
                    break;
                case ('n'):    /* don't overwrite any files */
                    if (negative)
                        overwrite_none = FALSE, negative = 0;
                    else
                        overwrite_none = TRUE;
                    break;
                case ('o'):    /* OK to overwrite files without prompting */
                    if (negative) {
                        overwrite_all = MAX(overwrite_all-negative,0);
                        force_flag = MAX(force_flag-negative,0);
                        negative = 0;
                    } else {
                        ++overwrite_all;
                        ++force_flag;  /* (share -o for now) force to cont. */
                    }
                    break;
                case ('p'):    /* pipes:  stdout, no translation, no messages */
                    if (negative) {
                        cflag = FALSE;
                        qflag = MAX(qflag-999,0);
                        negative = 0;
                    } else {
                        aflag = 0;
                        cflag = TRUE;
                        qflag += 999;
                    }
                    break;
                case ('q'):    /* quiet:  fewer comments/messages */
                    if (negative) {
                        qflag = MAX(qflag-negative,0);
                        negative = 0;
                    } else
                        ++qflag;
                    break;
#ifdef DOS_NT_OS2
                case ('s'):    /* spaces in filenames:  allow by default */
                    if (negative)
                        sflag = FALSE, negative = 0;
                    else
                        sflag = TRUE;
                    break;
#endif
                case ('t'):
                    if (negative)
                        tflag = FALSE, negative = 0;
                    else
                        tflag = TRUE;
                    break;
                case ('U'):    /* allow Uppercase (don't make all lowercase) */
                    if (negative)
                        U_flag = FALSE, negative = 0;
                    else
                        U_flag = TRUE;
                    break;
                case ('u'):    /* update (extract only new and newer files) */
                    if (negative)
                        uflag = FALSE, negative = 0;
                    else
                        uflag = TRUE;
                    break;
                case ('V'):    /* Version (retain VMS/DEC-20 file versions) */
                    if (negative)
                        V_flag = FALSE, negative = 0;
                    else
                        V_flag = TRUE;
                    break;
                case ('v'):    /* verbose */
                    if (negative) {
                        vflag = MAX(vflag-negative,0);
                        negative = 0;
                    } else if (vflag)
                        ++vflag;
                    else
                        vflag = 2;
                    break;
#ifdef VMS
                case ('X'):   /* restore owner/protection info (need privs?) */
                    if (negative)
                        secinf = FALSE, negative = 0;
                    else
                        secinf = TRUE;
                    break;
#endif /* VMS */
                case ('x'):    /* extract:  default */
                    break;
                case ('z'):    /* display only the archive comment */
                    if (negative) {
                        zflag -= negative;
                        negative = 0;
                    } else
                        ++zflag;
                    break;
#ifdef DOS_NT_OS2
                case ('$'):
                    if (negative) {
                        volflag = MAX(volflag-negative,0);
                        negative = 0;
                    } else
                        ++volflag;
                    break;
#endif
                default:
                    error = TRUE;
                    break;
            } /* end switch */
        } /* end while (not end of argument string) */
    } /* end while (not done with switches) */
    if ((cflag && tflag) || (cflag && uflag) || (cflag && vflag) ||
        (tflag && uflag) || (tflag && vflag) || (uflag && vflag) ||
        (fflag && overwrite_none)) {
        fprintf(stderr, "error:\
  -fn or any combination of -c, -l, -p, -t, -u and -v options invalid\n");
        error = TRUE;
    }
    if (aflag > 2)
        aflag = 2;
#if 0
    if ((aflag && tflag) || (aflag && vflag))   /* -a makes no sense */
        aflag = 0;
#endif
    if (overwrite_all && overwrite_none) {
        fprintf(stderr, "caution:  both -n and -o specified; ignoring -o\n");
        overwrite_all = FALSE;
    }
#ifdef SFX
    if (error)
#else
    if ((argc-- == 0) || error)
#endif
    {
        *pargc = argc;
        *pargv = argv;
        return usage(error);
    }
    if (cflag || tflag || vflag)
        extract_flag = FALSE;
    else
        extract_flag = TRUE;
    *pargc = argc;
    *pargv = argv;
    return 0;
} /* end function uz_opts() */
#ifdef SFX
int usage(error)   /* return PK-type error code */
    int error;
{
    FILE *usagefp;
    if (error)
        usagefp = (FILE *)stderr;
    else
        usagefp = (FILE *)stdout;
    fprintf(usagefp, "\
UnZipSFX %s, by Info-ZIP (zip-bugs@wkuvx1.wku.edu).\n\n",
      UZ_VERSION);
#ifndef RELEASE
    fprintf(usagefp, "\
    THIS IS STILL A BETA VERSION OF UNZIPSFX -- DO NOT DISTRIBUTE.\n\n");
#endif
    if (error)
        return PK_PARAM;
    else
        return PK_COOL;     /* just wanted usage screen: no error */
} /* end function usage() */
#else /* !SFX */
int usage(error)   /* return PK-type error code */
    int error;
{
#ifdef VMS
#  define QUOT '\"'
#  define QUOTS "\""
#  define EXAMPLE2 "vms.c"
#  define EXAMPLE1 \
"unzip \"-V\" foo \"Bar\" => must quote uppercase options and filenames in VMS"
#else
#  define QUOT ' '
#  define QUOTS ""
#  define EXAMPLE2 "ReadMe"
#  define EXAMPLE1 \
"unzip -p foo | more  => send contents of foo.zip via pipe into program more"
#endif
#ifdef DOS_NT_OS2
    char *loc_str = " -$  label removables (-$$ => fixed disks)";
    char *loc2str = "\
                                             -s  spaces in filenames => '_'\n";
#else /* !DOS_NT_OS2 */
#ifdef VMS
    char *loc_str = "\"-X\" restore owner/protection info";
    char *loc2str = "\n";
#else
    char *loc_str = "";   /* Unix, Amiga, Mac, etc. */
    char *loc2str = "";
#endif
#endif /* ?DOS_NT_OS2 */
    FILE *usagefp;
    if (error)
        usagefp = (FILE *)stderr;
    else
        usagefp = (FILE *)stdout;
    if (zipinfo_mode) {
#ifndef NO_ZIPINFO
        fprintf(usagefp, "\
   Usage:  UNZIP -Z [-12smlvhtz] file[.zip] [list...] [-x xlist...]\n");
        fprintf(usagefp, "\nmain\
 listing-format options:             -s  short Unix \"ls -l\" format (def.)\n\
  -1  filenames ONLY, one per line       -m  medium Unix \"ls -l\" format\n\
  -2  just filenames but allow -h/-t/-z  -l  long Unix \"ls -l\" format\n\
                                         -v  verbose, multi-page format\n\
miscellaneous options:\n  \
-h  print header line      -t  print totals for files listed or for all files\n\
  -z  print zipfile comment  -x  exclude filenames that follow from listing\n");
#ifdef VMS
        fprintf(usagefp, "\nRemember that non-lowercase filespecs must be\
 quoted in VMS (e.g., \"Makefile\").\n");
#endif
#endif /* !NO_ZIPINFO */
    } else {   /* UnZip mode */
        fprintf(usagefp, "\
Usage: UNZIP %s[-opts[modifiers]] file[.zip] [list] [-x xlist] [-d exdir]\n \
 Default action is to extract files in list, except those in xlist, to exdir;\n\
  file[.zip] may be a wildcard.  %s\n\n",
#ifdef NO_ZIPINFO
          "", "(ZipInfo mode is disabled in this version.)");
#else
#ifdef VMS
          "[-Z] ", "\"-Z\" => ZipInfo mode (`unzip \"-Z\"' for usage).");
#else
          "[-Z] ", "-Z => ZipInfo mode (\"unzip -Z\" for usage).");
#endif
#endif /* ?NO_ZIPINFO */
        fprintf(usagefp, "\
  -c  extract files to stdout/screen (CRT)   -l  list files (short format)\n\
  -p  extract files to pipe, no messages     -v  list files (verbose format)\n\
  -f  freshen existing files, create none    -t  test compressed archive data\n\
  -u  update files, create if necessary      -z  display archive comment\n\
  -x  exclude files which follow (in xlist)  -d  extract files into exdir\n\n");
        fprintf(usagefp, "\
modifiers:                                   -q  quiet mode (-qq => quieter)\n\
  -n  never overwrite existing files         -a  auto-convert any text files\n\
  -o  overwrite files WITHOUT prompting      -aa treat ALL files as text\n\
  -j  junk paths (don't make directories)   %c-U%c don't make names lowercase\n\
 %-42s %c-V%c retain VMS version numbers\n%s",
          QUOT,QUOT, loc_str, QUOT,QUOT, loc2str);
    }
    if (error)
        return PK_PARAM;
    else
        return PK_COOL;     /* just wanted usage screen: no error */
} /* end function usage() */
#endif /* ?SFX */
#endif /* ?MSWIN */
int process_zipfiles()    /* return PK-type error code */
{
    char *lastzipfn = NULL;
    int error=0, error_in_archive=0;
    int num_win_files, num_lose_files, num_warn_files;
    int num_miss_dirs, num_miss_files;
    local_hdr_sig[0]  /* = extd_local_sig[0] */  = '\120';   /* ASCII 'P', */
    central_hdr_sig[0] = end_central_sig[0] = '\120';        /* not EBCDIC */
    strcpy(local_hdr_sig+1, LOCAL_HDR_SIG);
    strcpy(central_hdr_sig+1, CENTRAL_HDR_SIG);
    strcpy(end_central_sig+1, END_CENTRAL_SIG);
#ifdef SFX
    if ((error = do_seekable(0)) != IZ_DIR && error > error_in_archive)
        error_in_archive = error;
#else /* !SFX */
    num_win_files = num_lose_files = num_warn_files = 0;
    num_miss_dirs = num_miss_files = 0;
    while ((zipfn = do_wild(wildzipfn)) != NULL) {
        Trace((stderr, "do_wild( %s ) returns %s\n", wildzipfn, zipfn));
        lastzipfn = zipfn;
        if (!qflag  &&  error != PK_NOZIP  &&
            (num_win_files+num_lose_files+num_warn_files+num_miss_files) > 0)
            printf("\n");
        FFLUSH(stdout);
        if ((error = do_seekable(0)) == PK_WARN)
            ++num_warn_files;
        else if (error == IZ_DIR)
            ++num_miss_dirs;
        else if (error == PK_NOZIP)
            ++num_miss_files;
        else if (error)
            ++num_lose_files;
        else
            ++num_win_files;
        if (error != IZ_DIR && error > error_in_archive)
            error_in_archive = error;
        Trace((stderr, "do_seekable(0) returns %d\n", error));
    } /* end while-loop (wildcard zipfiles) */
    if ((num_win_files + num_warn_files + num_lose_files) == 0  &&
        (num_miss_dirs + num_miss_files) == 1  &&  lastzipfn != NULL)
    {
        num_miss_dirs = num_miss_files = 0;
        if (error_in_archive == PK_NOZIP)
            error_in_archive = PK_COOL;
        strcat((zipfn = lastzipfn), ZSUFX);
        if ((error = do_seekable(1)) == PK_WARN)
            ++num_warn_files;
        else if (error == IZ_DIR)
            ++num_miss_dirs;
        else if (error == PK_NOZIP)
            ++num_miss_files;
        else if (error)
            ++num_lose_files;
        else
            ++num_win_files;
        if (error > error_in_archive)
            error_in_archive = error;
        Trace((stderr, "do_seekable(1) returns %d\n", error));
    }
#endif /* ?SFX */
    FFLUSH(stdout);
    FFLUSH(stderr);
#ifndef SFX
    if (num_miss_dirs + num_miss_files + num_lose_files + num_warn_files > 0
        || num_win_files != 1)
        fprintf(stderr, "\n");
    if ((num_win_files > 1) || (num_win_files == 1 &&
        num_miss_dirs + num_miss_files + num_lose_files + num_warn_files > 0))
        fprintf(stderr, "%d archive%s successfully processed.\n",
          num_win_files, (num_win_files == 1)? " was" : "s were");
    if (num_warn_files > 0)
        fprintf(stderr, "%d archive%s had warnings but no fatal errors.\n",
          num_warn_files, (num_warn_files == 1)? "" : "s");
    if (num_lose_files > 0)
        fprintf(stderr, "%d archive%s had fatal errors.\n",
          num_lose_files, (num_lose_files == 1)? "" : "s");
    if (num_miss_files > 0)
        fprintf(stderr, "%d file%s had no zipfile directory.\n",
          num_miss_files, (num_miss_files == 1)? "" : "s");
    if (num_miss_dirs == 1)
        fprintf(stderr, "1 \"zipfile\" was a directory.\n");
    else if (num_miss_dirs > 0)
        fprintf(stderr, "%d \"zipfiles\" were directories.\n", num_miss_dirs);
    if (num_win_files + num_lose_files + num_warn_files == 0)
        fprintf(stderr, "No zipfiles found.\n");
#endif /* !SFX */
    inflate_free();
    checkdir((char *)NULL, END);
#ifndef SMALL_MEM
    if (outbuf2)
        free(outbuf2);   /* malloc'd ONLY if unshrink and -a */
#endif
    free(outbuf);
    free(inbuf);
    return error_in_archive;
} /* end function process_zipfiles() */
int do_seekable(lastchance)    /* return PK-type error code */
    int lastchance;
{
    int error=0, error_in_archive, maybe_exe=FALSE;
    static int no_ecrec = FALSE;
    if (SSTAT(zipfn, &statbuf) || (error = S_ISDIR(statbuf.st_mode)) != 0) {
        if (lastchance)
            if (no_ecrec)
                fprintf(stderr, "%s:  can't find zipfile directory in %s,\n\
        %sand can't find %s, period.\n", zipinfo_mode? "zipinfo" : "unzip",
                  wildzipfn, zipinfo_mode? "  " : "", zipfn);
            else
                fprintf(stderr,
                  "%s:  can't find either %s or %s, so there.\n",
                  zipinfo_mode? "zipinfo" : "unzip", wildzipfn, zipfn);
        return error? IZ_DIR : PK_NOZIP;
    }
    ziplen = statbuf.st_size;
#if defined(UNIX) || defined(DOS_NT_OS2)  /* no extension on Unix exec's: */
    if (statbuf.st_mode & S_IEXEC)      /* might find zip, not zip.zip; etc. */
        maybe_exe = TRUE;               /* (for other OS's, some executables */
#endif                                  /* can be self-extracting archives) */
#ifdef VMS
    if (check_format())      /* check for variable-length format */
        return PK_ERR;
#endif
    if (open_input_file())   /* this should never happen, given */
        return PK_NOZIP;     /*  the stat() test above, but... */
    cur_zipfile_bufstart = 0;
    inptr = inbuf;
    if (!qflag && !zipinfo_mode)
        printf("Archive:  %s\n", zipfn);
    if ((
#ifndef NO_ZIPINFO
         zipinfo_mode &&
          ((error_in_archive = find_end_central_dir(ziplen)) != 0 ||
          (error_in_archive = zi_end_central()) > PK_WARN))
        || (!zipinfo_mode &&
#endif
          ((error_in_archive = find_end_central_dir(MIN(ziplen,66000L))) != 0 ||
          (error_in_archive = uz_end_central()) > PK_WARN)))
    {
        close(zipfd);
#ifdef SFX
        return error_in_archive;
#else
        if (maybe_exe)
            fprintf(stderr, "note:  %s may be an executable, not an archive\n",
              zipfn);
        if (lastchance)
            return error_in_archive;
        else {
            no_ecrec = TRUE;    /* assume we found wrong file:  e.g., */
            return PK_NOZIP;    /*  unzip instead of unzip.zip */
        }
#endif /* ?SFX */
    }
    if ((zflag > 0) && !zipinfo_mode) {   /* in unzip, zflag = comment ONLY */
        close(zipfd);
        return error_in_archive;
    }
    error = !zipinfo_mode && (ecrec.number_this_disk == 1) &&
            (ecrec.num_disk_with_start_central_dir == 1);
    if (zipinfo_mode &&
        ecrec.number_this_disk != ecrec.num_disk_with_start_central_dir)
    {
        fprintf(stderr, "\n\
     Zipfile is part of a multi-disk archive, and this is not the disk on\n\
     which the central zipfile directory begins.\n");
        error_in_archive = PK_FIND;
    } else if (!zipinfo_mode && !error && ecrec.number_this_disk != 0) {
        fprintf(stderr, "\nerror [%s]:  zipfile is part of multi-disk archive\n\
  (sorry, not yet supported).\n", zipfn);
        error_in_archive = PK_FIND;
    } else {   /* this is a (relatively) normal zipfile:  process normally */
        if (error) {
            fprintf(stderr, "\n\
     Warning:  zipfile claims to be second disk of a two-part archive;\n\
     attempting to process anyway.)\n\n");
            error_in_archive = PK_WARN;
        }
        if ((extra_bytes = real_ecrec_offset - expect_ecrec_offset) < (LONGINT)0) {
            fprintf(stderr, "error [%s]:  missing %ld bytes in zipfile\n\
  (attempting to process anyway)\n", zipfn, (long)(-extra_bytes));
            error_in_archive = PK_ERR;
        } else if (extra_bytes > 0) {
            if ((ecrec.offset_start_central_directory == 0) &&
                (ecrec.size_central_directory != 0))   /* zip 1.5 -go bug */
            {
                fprintf(stderr, "error [%s]:  NULL central directory offset\n\
  (attempting to process anyway)\n", zipfn);
                ecrec.offset_start_central_directory = extra_bytes;
                extra_bytes = 0;
                error_in_archive = PK_ERR;
            } else {
#ifndef SFX
                fprintf(stderr, "warning [%s]:  extra %ld bytes at beginning\
 or within zipfile\n  (attempting to process anyway)\n", zipfn,
                  (long)extra_bytes);
                error_in_archive = PK_WARN;
#endif
            }
        }
        if (expect_ecrec_offset == 0L  &&  ecrec.size_central_directory == 0) {
            if (zipinfo_mode)
                printf("%sEmpty zipfile.\n", lflag>9 ? "\n  " : "");
            else
                fprintf(stderr, "warning [%s]:  zipfile is empty\n", zipfn);
            close(zipfd);
            return (error_in_archive > PK_WARN)? error_in_archive : PK_WARN;
        }
        LSEEK( ecrec.offset_start_central_directory )
        if ((error = readbuf(sig, 4)) < 0) {
            close(zipfd);
            return PK_ERR;  /* file may be locked, or possibly disk error(?) */
        }
        if ((error == 0) || strncmp(sig, central_hdr_sig, 4)) {
            long tmp = extra_bytes;
            extra_bytes = 0;
            LSEEK( ecrec.offset_start_central_directory )
            if ((readbuf(sig, 4) <= 0) || strncmp(sig, central_hdr_sig, 4)) {
                fprintf(stderr,
                  "error [%s]:  start of central directory not found;\n\
  zipfile corrupt.\n%s", zipfn, ReportMsg);
                close(zipfd);
                return PK_BADERR;
            }
            fprintf(stderr,
              "error [%s]:  reported length of central directory is\n\
  %ld bytes too long (Atari STZIP zipfile?  J.H.Holm ZIPSPLIT 1.1\n\
  zipfile?).  Compensating...\n", zipfn, -tmp);
            error_in_archive = PK_ERR;
        }
        Trace((stderr, "about to extract/list files (error = %d)\n",
          error_in_archive));
        LSEEK( ecrec.offset_start_central_directory )
#ifndef NO_ZIPINFO
        if (zipinfo_mode) {
            error = zipinfo();                     /* ZIPINFO 'EM */
            if (lflag > 9)
                printf("\n");
        } else
#endif
            if (vflag)
                error = list_files();              /* LIST 'EM */
            else
                error = extract_or_test_files();   /* EXTRACT OR TEST 'EM */
        Trace((stderr, "done with extract/list files (error = %d)\n", error));
        if (error > error_in_archive)   /* don't overwrite stronger error */
            error_in_archive = error;   /*  with (for example) a warning */
    }
    close(zipfd);
    return error_in_archive;
} /* end function do_seekable() */
int uz_end_central()    /* return PK-type error code */
{
    int error = PK_COOL;
#ifdef MSWIN
    cchComment = ecrec.zipfile_comment_length; /* save for comment button */
    if (ecrec.zipfile_comment_length && (zflag > 0))
#else /* !MSWIN */
    if (ecrec.zipfile_comment_length && (zflag > 0 || (zflag == 0 && !qflag)))
#endif /* ?MSWIN */
    {
#if 0
#ifndef MSWIN
        if (zflag == 0)       (add "&& single_zipfile" perhaps; unnecessary with
            printf("[%s] comment:\n", zipfn);  multiple zipfiles: "Archive:...")
#endif /* !MSWIN */
#endif /* 0 */
        if (do_string(ecrec.zipfile_comment_length,DISPLAY)) {
            fprintf(stderr, "\ncaution:  zipfile comment truncated\n");
            error = PK_WARN;
        }
    }
    return error;
} /* end function uz_end_central() */
int process_cdir_file_hdr()    /* return PK-type error code */
{
    int error;
    if ((error = get_cdir_file_hdr()) != 0)
        return error;
    pInfo->hostnum = MIN(crec.version_made_by[1], NUM_HOSTS);
    pInfo->lcflag = 0;
    if (!U_flag)   /* as long as user hasn't specified case-preservation */
        switch (pInfo->hostnum) {
            case FS_FAT_:     /* PKZIP and zip -k store in uppercase */
            case ATARI_:      /* MS-DOS filesystem */
            case CPM_:        /* like MS-DOS, right? */
            case VM_CMS_:     /* all caps? */
            case TOPS20_:
                pInfo->lcflag = 1;   /* convert filename to lowercase */
                break;
            default:     /* AMIGA_, FS_HPFS_, FS_NTFS_, MAC_, UNIX_, */
                break;   /*  (Z_SYSTEM_):  no conversion */
        }
    if (IS_VOLID(crec.external_file_attributes) &&
        (pInfo->hostnum == FS_FAT_ || pInfo->hostnum == FS_HPFS_ ||
         pInfo->hostnum == FS_NTFS_ || pInfo->hostnum == ATARI_))
    {
        pInfo->vollabel = TRUE;
        pInfo->lcflag = 0;        /* preserve case of volume labels */
    } else
        pInfo->vollabel = FALSE;
    return PK_COOL;
} /* end function process_cdir_file_hdr() */
int process_local_file_hdr()    /* return PK-type error code */
{
    local_byte_hdr byterec;
    if (readbuf((char *)byterec, LREC_SIZE) <= 0)
        return PK_EOF;
    lrec.version_needed_to_extract[0] = byterec[L_VERSION_NEEDED_TO_EXTRACT_0];
    lrec.version_needed_to_extract[1] = byterec[L_VERSION_NEEDED_TO_EXTRACT_1];
    lrec.general_purpose_bit_flag = makeword(&byterec[L_GENERAL_PURPOSE_BIT_FLAG]);
    lrec.compression_method = makeword(&byterec[L_COMPRESSION_METHOD]);
    lrec.last_mod_file_time = makeword(&byterec[L_LAST_MOD_FILE_TIME]);
    lrec.last_mod_file_date = makeword(&byterec[L_LAST_MOD_FILE_DATE]);
    lrec.crc32 = makelong(&byterec[L_CRC32]);
    lrec.csize = makelong(&byterec[L_COMPRESSED_SIZE]);
    lrec.ucsize = makelong(&byterec[L_UNCOMPRESSED_SIZE]);
    lrec.filename_length = makeword(&byterec[L_FILENAME_LENGTH]);
    lrec.extra_field_length = makeword(&byterec[L_EXTRA_FIELD_LENGTH]);
    csize = (long) lrec.csize;
    ucsize = (long) lrec.ucsize;
    if ((lrec.general_purpose_bit_flag & 8) != 0) {
        lrec.crc32 = pInfo->crc;
        csize = (long)(lrec.csize = pInfo->compr_size);
    }
    return PK_COOL;
} /* end function process_local_file_hdr() */
