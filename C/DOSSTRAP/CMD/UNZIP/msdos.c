/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
#undef FILENAME	    
static int isfloppy OF((int nDrive));
static int volumelabel OF((char *newlabel));
static int created_dir;        /* used by mapname(), checkdir() */
static int renamed_fullpath;   /* ditto */
static unsigned nLabelDrive;   /* ditto, plus volumelabel() */
#if (defined(__GO32__) || defined(__EMX__))
#  define MKDIR(path,mode)   mkdir(path,mode)
#  include <dirent.h>        /* use readdir() */
#  define direct dirent
#  define Opendir opendir
#  define Readdir readdir
#  ifdef __EMX__
#    include <dos.h>
#    define GETDRIVE(d)      d = _getdrive()
#    define FA_LABEL         A_LABEL
#  else
#    define GETDRIVE(d)      _dos_getdrive(&d)
#  endif
#else /* !(__GO32__ || __EMX__) */
#  define MKDIR(path,mode)   mkdir(path)
#  ifdef __TURBOC__
#    define FATTR            FA_HIDDEN+FA_SYSTEM+FA_DIREC
#    define FVOLID           FA_VOLID
#    define FFIRST(n,d,a)    findfirst(n,(struct ffblk *)d,a)
#    define FNEXT(d)         findnext((struct ffblk *)d)
#    define GETDRIVE(d)      d=getdisk()+1
#    include <dir.h>
#  else /* !__TURBOC__ */
#    define FATTR            _A_HIDDEN+_A_SYSTEM+_A_SUBDIR
#    define FVOLID           _A_VOLID
#    define FFIRST(n,d,a)    _dos_findfirst(n,a,(struct find_t *)d)
#    define FNEXT(d)         _dos_findnext((struct find_t *)d)
#    define GETDRIVE(d)      _dos_getdrive(&d)
#    include <direct.h>
#  endif /* ?__TURBOC__ */
   typedef struct direct {
       char d_reserved[30];
       char d_name[13];
       int d_first;
   } DIR;
#  define closedir free
   DIR *Opendir OF((const char *));
   struct direct *Readdir OF((DIR *));
DIR *Opendir(name)
    const char *name;        /* name of directory to open */
{
    DIR *dirp;               /* malloc'd return value */
    char *nbuf;              /* malloc'd temporary string */
    int len = strlen(name);  /* path length to avoid strlens and strcats */
    if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL)
        return NULL;
    if ((nbuf = malloc(len + 5)) == NULL) {
        free(dirp);
        return NULL;
    }
    strcpy(nbuf, name);
    if (nbuf[len-1] == ':') {
        nbuf[len++] = '.';
    } else if (nbuf[len-1] == '/' || nbuf[len-1] == '\\')
        --len;
    strcpy(nbuf+len, "/*.*");
    Trace((stderr, "opendir:  nbuf = [%s]\n", nbuf));
    if (FFIRST(nbuf, dirp, FATTR)) {
        free((voidp *)nbuf);
        return NULL;
    }
    free((voidp *)nbuf);
    dirp->d_first = 1;
    return dirp;
}
struct direct *Readdir(d)
    DIR *d;         /* directory stream from which to read */
{
    if (d->d_first)
        d->d_first = 0;
    else
        if (FNEXT(d))
            return NULL;
    return (struct direct *)d;
}
#endif /* ?(__GO32__ || __EMX__) */
char *do_wild(wildspec)
    char *wildspec;          /* only used first time on a given dir */
{
    static DIR *dir = NULL;
    static char *dirname, *wildname, matchname[FILNAMSIZ];
    static int firstcall=TRUE, have_dirname, dirnamelen;
    struct direct *file;
    if (firstcall) {        /* first call:  must initialize everything */
        firstcall = FALSE;
        if ((wildname = strrchr(wildspec, '/')) == NULL &&
            (wildname = strrchr(wildspec, ':')) == NULL) {
            dirname = ".";
            dirnamelen = 1;
            have_dirname = FALSE;
            wildname = wildspec;
        } else {
            ++wildname;     /* point at character after '/' or ':' */
            dirnamelen = wildname - wildspec;
            if ((dirname = (char *)malloc(dirnamelen+1)) == NULL) {
                fprintf(stderr, "warning:  can't allocate wildcard buffers\n");
                strcpy(matchname, wildspec);
                return matchname;   /* but maybe filespec was not a wildcard */
            }
            strncpy(dirname, wildspec, dirnamelen);
            dirname[dirnamelen] = '\0';   /* terminate for strcpy below */
            have_dirname = TRUE;
        }
        Trace((stderr, "do_wild:  dirname = [%s]\n", dirname));
        if ((dir = Opendir(dirname)) != NULL) {
            while ((file = Readdir(dir)) != NULL) {
                Trace((stderr, "do_wild:  readdir returns %s\n", file->d_name));
                if (match(file->d_name, wildname, 1)) {  /* 1 == ignore case */
                    Trace((stderr, "do_wild:  match() succeeds\n"));
                    if (have_dirname) {
                        strcpy(matchname, dirname);
                        strcpy(matchname+dirnamelen, file->d_name);
                    } else
                        strcpy(matchname, file->d_name);
                    return matchname;
                }
            }
            closedir(dir);
            dir = NULL;
        }
        Trace((stderr, "do_wild:  opendir(%s) returns NULL\n", dirname));
        strcpy(matchname, wildspec);
        return matchname;
    }
    if (dir == NULL) {
        firstcall = TRUE;  /* nothing left to try--reset for new wildspec */
        if (have_dirname)
            free(dirname);
        return (char *)NULL;
    }
    while ((file = Readdir(dir)) != NULL)
        if (match(file->d_name, wildname, 1)) {   /* 1 == ignore case */
            if (have_dirname) {
                strcpy(matchname+dirnamelen, file->d_name);
            } else
                strcpy(matchname, file->d_name);
            return matchname;
        }
    closedir(dir);     /* have read at least one dir entry; nothing left */
    dir = NULL;
    firstcall = TRUE;  /* reset for new wildspec */
    if (have_dirname)
        free(dirname);
    return (char *)NULL;
} /* end function do_wild() */
int mapattr()
{
    pInfo->file_attr = (unsigned)(crec.external_file_attributes | 32) & 0xff;
    return 0;
} /* end function mapattr() */
int mapname(renamed)  /* return 0 if no error, 1 if caution (filename trunc), */
    int renamed;      /* 2 if warning (skip file because dir doesn't exist), */
{                     /* 3 if error (skip file), 10 if no memory (skip file) */
    char pathcomp[FILNAMSIZ];   /* path-component buffer */
    char *pp, *cp=NULL;         /* character pointers */
    char *lastsemi = NULL;      /* pointer to last semi-colon in pathcomp */
    char *last_dot = NULL;      /* last dot not converted to underscore */
    int quote = FALSE;          /* flag:  next char is literal */
    int dotname = FALSE;        /* flag:  path component begins with dot */
    int error = 0;
    register unsigned workch;   /* hold the character being tested */
    create_dirs = (!fflag || renamed);
    created_dir = FALSE;        /* not yet */
    renamed_fullpath = FALSE;
    if (renamed) {
        cp = filename - 1;      /* point to beginning of renamed name... */
        while (*++cp)
            if (*cp == '\\')    /* convert backslashes to forward */
                *cp = '/';
        cp = filename;
        if (filename[0] == '/') {
            renamed_fullpath = TRUE;
            pathcomp[0] = '/';  /* copy the '/' and terminate */
            pathcomp[1] = '\0';
            ++cp;
        } else if (isalpha(filename[0]) && filename[1] == ':') {
            renamed_fullpath = TRUE;
            pp = pathcomp;
            *pp++ = *cp++;      /* copy the "d:" (+ '/', possibly) */
            *pp++ = *cp++;
            if (*cp == '/')
                *pp++ = *cp++;  /* otherwise add "./"? */
            *pp = '\0';
        }
    }
    if ((error = checkdir(pathcomp, INIT)) != 0)    /* initialize path buffer */
        return error;           /* ...unless no mem or vol label on hard disk */
    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (!renamed) {             /* cp already set if renamed */
        if (jflag)              /* junking directories */
            cp = (char *)strrchr(filename, '/');
        if (cp == NULL)         /* no '/' or not junking dirs */
            cp = filename;      /* point to internal zipfile-member pathname */
        else
            ++cp;               /* point to start of last component of path */
    }
    while ((workch = (uch)*cp++) != 0) {
        if (quote) {              /* if character quoted, */
            *pp++ = (char)workch; /*  include it literally */
            quote = FALSE;
        } else
            switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if (last_dot) {   /* one dot in directory name is legal */
                    *last_dot = '.';
                    last_dot = NULL;
                }
                if ((error = checkdir(pathcomp, APPEND_DIR)) > 1)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = NULL;  /* leave directory semi-colons alone */
                break;
            case ':':             /* drive names not stored in zipfile, */
            case '[':             /*  so no colons allowed; no brackets, */
            case ']':             /*  either */
                *pp++ = '_';
                break;
            case '.':
                if (pp == pathcomp) {     /* nothing appended yet... */
                    if (*cp == '/') {     /* don't bother appending a "./" */
                        ++cp;             /*  component to the path:  skip */
                        break;            /*  to next char after the '/' */
                    } else if (*cp == '.' && cp[1] == '/') {   /* "../" */
                        *pp++ = '.';      /* add first dot, unchanged... */
                        ++cp;             /* skip second dot, since it will */
                    } else {              /*  be "added" at end of if-block */
                        *pp++ = '_';      /* FAT doesn't allow null filename */
                        dotname = TRUE;   /*  bodies, so map .exrc -> _.exrc */
                    }                     /*  (extra '_' now, "dot" below) */
                } else if (dotname) {     /* found a second dot, but still */
                    dotname = FALSE;      /*  have extra leading underscore: */
                    *pp = '\0';           /*  remove it by shifting chars */
                    pp = pathcomp + 1;    /*  left one space (e.g., .p1.p2: */
                    while (pp[1]) {       /*  __p1 -> _p1_p2 -> _p1.p2 when */
                        *pp = pp[1];      /*  finished) [opt.:  since first */
                        ++pp;             /*  two chars are same, can start */
                    }                     /*  shifting at second position] */
                }
                last_dot = pp;    /* point at last dot so far... */
                *pp++ = '_';      /* convert dot to underscore for now */
                break;
            case ';':             /* start of VMS version? */
                lastsemi = pp;    /* omit for now; remove VMS vers. later */
                break;
            case '\026':          /* control-V quote for special chars */
                quote = TRUE;     /* set flag for next character */
                break;
            case ' ':             /* change spaces to underscore only */
                if (sflag)        /*  if specifically requested */
                    *pp++ = '_';
                else
                    *pp++ = (char)workch;
                break;
            default:
                if (isprint(workch) || (128 <= workch && workch <= 254))
                    *pp++ = (char)workch;
            } /* end switch */
    } /* end while loop */
    *pp = '\0';                   /* done with pathcomp:  terminate it */
    if (!V_flag && lastsemi) {
        pp = lastsemi;            /* semi-colon was omitted:  expect all #'s */
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }
    if (last_dot != NULL)         /* one dot is OK:  put it back in */
        *last_dot = '.';          /* (already done for directories) */
    if (filename[strlen(filename) - 1] == '/') {
        checkdir(filename, GETPATH);
        if (created_dir && QCOND2) {
            fprintf(stdout, "   creating: %s\n", filename);
            return IZ_CREATED_DIR;   /* set dir time (note trailing '/') */
        }
        return 2;   /* dir existed already; don't look for data to extract */
    }
    if (*pathcomp == '\0') {
        fprintf(stderr, "mapname:  conversion of %s failed\n", filename);
        return 3;
    }
    checkdir(pathcomp, APPEND_NAME);   /* returns 1 if truncated:  care? */
    checkdir(filename, GETPATH);
    if (pInfo->vollabel) {   /* set the volume label now */
        if (QCOND2)
            fprintf(stdout, "labelling %c: %-22s\n", (nLabelDrive + 'a' - 1),
              filename);
        if (volumelabel(filename)) {
            fprintf(stderr, "mapname:  error setting volume label\n");
            return 3;
        }
        return 2;   /* success:  skip the "extraction" quietly */
    }
    return error;
} /* end function mapname() */
int checkdir(pathcomp, flag)
    char *pathcomp;
    int flag;
{
    static int rootlen = 0;   /* length of rootpath */
    static char *rootpath;    /* user's "extract-to" directory */
    static char *buildpath;   /* full path (so far) to extracted file */
    static char *end;         /* pointer to end of buildpath ('\0') */
#ifdef MSC
    int attrs;                /* work around MSC stat() bug */
#endif
#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)
    if (FUNCTION == APPEND_DIR) {
        int too_long = FALSE;
        Trace((stderr, "appending dir segment [%s]\n", pathcomp));
        while ((*end = *pathcomp++) != '\0')
            ++end;
        if ((end-buildpath) > FILNAMSIZ-3)  /* need '/', one-char name, '\0' */
            too_long = TRUE;                /* check if extracting directory? */
#ifdef MSC /* MSC 6.00 bug:  stat(non-existent-dir) == 0 [exists!] */
        if (_dos_getfileattr(buildpath, &attrs) || stat(buildpath, &statbuf))
#else
        if (stat(buildpath, &statbuf))      /* path doesn't exist */
#endif
        {
            if (!create_dirs) {   /* told not to create (freshening) */
                free(buildpath);
                return 2;         /* path doesn't exist:  nothing to do */
            }
            if (too_long) {
                fprintf(stderr, "checkdir error:  path too long: %s\n",
                  buildpath);
                fflush(stderr);
                free(buildpath);
                return 4;         /* no room for filenames:  fatal */
            }
            if (MKDIR(buildpath, 0777) == -1) {   /* create the directory */
                fprintf(stderr, "checkdir error:  can't create %s\n\
                 unable to process %s.\n", buildpath, filename);
                fflush(stderr);
                free(buildpath);
                return 3;      /* path didn't exist, tried to create, failed */
            }
            created_dir = TRUE;
        } else if (!S_ISDIR(statbuf.st_mode)) {
            fprintf(stderr, "checkdir error:  %s exists but is not directory\n\
                 unable to process %s.\n", buildpath, filename);
            fflush(stderr);
            free(buildpath);
            return 3;          /* path existed but wasn't dir */
        }
        if (too_long) {
            fprintf(stderr, "checkdir error:  path too long: %s\n", buildpath);
            fflush(stderr);
            free(buildpath);
            return 4;         /* no room for filenames:  fatal */
        }
        *end++ = '/';
        *end = '\0';
        Trace((stderr, "buildpath now = [%s]\n", buildpath));
        return 0;
    } /* end if (FUNCTION == APPEND_DIR) */
    if (FUNCTION == GETPATH) {
        strcpy(pathcomp, buildpath);
        Trace((stderr, "getting and freeing path [%s]\n", pathcomp));
        free(buildpath);
        buildpath = end = NULL;
        return 0;
    }
    if (FUNCTION == APPEND_NAME) {
        Trace((stderr, "appending filename [%s]\n", pathcomp));
        while ((*end = *pathcomp++) != '\0') {
            ++end;
            if ((end-buildpath) >= FILNAMSIZ) {
                *--end = '\0';
                fprintf(stderr, "checkdir warning:  path too long; truncating\n\
checkdir warning:  path too long; truncating\n\
                   %s\n                -> %s\n", filename, buildpath);
                fflush(stderr);
                return 1;   /* filename truncated */
            }
        }
        Trace((stderr, "buildpath now = [%s]\n", buildpath));
        return 0;  /* could check for existence here, prompt for new name... */
    }
    if (FUNCTION == INIT) {
        Trace((stderr, "initializing buildpath to "));
        if ((buildpath = (char *)malloc(strlen(filename)+rootlen+1)) == NULL)
            return 10;
        if (pInfo->vollabel) {
            if (renamed_fullpath && pathcomp[1] == ':')
                *buildpath = ToLower(*pathcomp);
            else if (!renamed_fullpath && rootpath && rootpath[1] == ':')
                *buildpath = ToLower(*rootpath);
            else {
                GETDRIVE(nLabelDrive);   /* assumed that a == 1, b == 2, etc. */
                *buildpath = (char)(nLabelDrive - 1 + 'a');
            }
            nLabelDrive = *buildpath - 'a' + 1;       /* save for mapname() */
            if (volflag == 0 || *buildpath < 'a' ||   /* no labels/bogus disk */
                (volflag == 1 && !isfloppy(nLabelDrive))) {  /* -$:  no fixed */
                free(buildpath);
                return IZ_VOL_LABEL;     /* skipping with message */
            }
            *buildpath = '\0';
            end = buildpath;
        } else if (renamed_fullpath) {   /* pathcomp = valid data */
            end = buildpath;
            while ((*end = *pathcomp++) != '\0')
                ++end;
        } else if (rootlen > 0) {
            strcpy(buildpath, rootpath);
            end = buildpath + rootlen;
        } else {
            *buildpath = '\0';
            end = buildpath;
        }
        Trace((stderr, "[%s]\n", buildpath));
        return 0;
    }
    if (FUNCTION == ROOT) {
        Trace((stderr, "initializing root path to [%s]\n", pathcomp));
        if (pathcomp == NULL) {
            rootlen = 0;
            return 0;
        }
        if ((rootlen = strlen(pathcomp)) > 0) {
            int had_trailing_pathsep=FALSE, has_drive=FALSE, xtra=2;
            if (isalpha(pathcomp[0]) && pathcomp[1] == ':')
                has_drive = TRUE;   /* drive designator */
            if (pathcomp[rootlen-1] == '/') {
                pathcomp[--rootlen] = '\0';
                had_trailing_pathsep = TRUE;
            }
            if (has_drive && (rootlen == 2)) {
                if (!had_trailing_pathsep)   /* i.e., original wasn't "x:/" */
                    xtra = 3;      /* room for '.' + '/' + 0 at end of "x:" */
            } else {               /* don't bother checking "x:." and "x:/" */
#ifdef MSC /* MSC 6.00 bug:  stat(non-existent-dir) == 0 [exists!] */
                if (_dos_getfileattr(pathcomp, &attrs) ||
                    SSTAT(pathcomp, &statbuf) || !S_ISDIR(statbuf.st_mode))
#else
                if (SSTAT(pathcomp, &statbuf) || !S_ISDIR(statbuf.st_mode))
#endif
                {
                    if (!create_dirs
#ifdef OLD_EXDIR
                                     || (!has_drive && !had_trailing_pathsep)
#endif
                                                                             ) {
                        rootlen = 0;
                        return 2;   /* treat as stored file */
                    }
                    if (MKDIR(pathcomp, 0777) == -1) {
                        fprintf(stderr,
                          "checkdir:  can't create extraction directory: %s\n",
                          pathcomp);
                        fflush(stderr);
                        rootlen = 0;   /* path didn't exist, tried to create, */
                        return 3;  /* failed:  file exists, or need 2+ levels */
                    }
                }
            }
            if ((rootpath = (char *)malloc(rootlen+xtra)) == NULL) {
                rootlen = 0;
                return 10;
            }
            strcpy(rootpath, pathcomp);
            if (xtra == 3)                  /* had just "x:", make "x:." */
                rootpath[rootlen++] = '.';
            rootpath[rootlen++] = '/';
            rootpath[rootlen] = '\0';
        }
        Trace((stderr, "rootpath now = [%s]\n", rootpath));
        return 0;
    }
    if (FUNCTION == END) {
        Trace((stderr, "freeing rootpath\n"));
        if (rootlen > 0)
            free(rootpath);
        return 0;
    }
    return 99;  /* should never reach */
} /* end function checkdir() */
static int isfloppy(nDrive)  /* more precisely, is it removable? */
    int nDrive;
{
    union REGS regs;
    regs.h.ah = 0x44;
    regs.h.al = 0x08;
    regs.h.bl = (uch)nDrive;
#ifdef __EMX__
    _int86(0x21, &regs, &regs);
    if (regs.x.flags & 1)
#else
    intdos(&regs, &regs);
    if (regs.x.cflag)        /* error:  do default a/b check instead */
#endif
    {
        Trace((stderr,
          "error in DOS function 0x44 (AX = 0x%04x):  guessing instead...\n",
          regs.x.ax));
        return (nDrive == 1 || nDrive == 2)? TRUE : FALSE;
    } else
        return regs.x.ax? FALSE : TRUE;
}
#if (!defined(__GO32__) && !defined(__EMX__))
typedef struct dosfcb {
    uch  flag;        /* ff to indicate extended FCB */
    char res[5];      /* reserved */
    uch  vattr;       /* attribute */
    uch  drive;       /* drive (1=A, 2=B, ...) */
    uch  vn[11];      /* file or volume name */
    char dmmy[5];
    uch  nn[11];      /* holds new name if renaming (else reserved) */
    char dmmy2[9];
} dos_fcb;
static int volumelabel(newlabel)
    char *newlabel;
{
#ifdef DEBUG
    char *p;
#endif
    int len = strlen(newlabel);
    dos_fcb  fcb, dta, far *pfcb=&fcb, far *pdta=&dta;
    struct SREGS sregs;
    union REGS regs;
    memset((char *)&dta, 0, sizeof(dos_fcb));
    memset((char *)&fcb, 0, sizeof(dos_fcb));
#ifdef DEBUG
    for (p = (char *)&dta; (p - (char *)&dta) < sizeof(dos_fcb); ++p)
        if (*p)
            fprintf(stderr, "error:  dta[%d] = %x\n", (p - (char *)&dta), *p);
    for (p = (char *)&fcb; (p - (char *)&fcb) < sizeof(dos_fcb); ++p)
        if (*p)
            fprintf(stderr, "error:  fcb[%d] = %x\n", (p - (char *)&fcb), *p);
    printf("testing pointer macros:\n");
    segread(&sregs);
    printf("cs = %x, ds = %x, es = %x, ss = %x\n", sregs.cs, sregs.ds, sregs.es,
      sregs.ss);
#endif /* DEBUG */
#if 0
#ifdef __TURBOC__
    bdosptr(0x1a, dta, DO_NOT_CARE);
#else
    (intdosx method below)
#endif
#endif /* 0 */
    sregs.ds = FP_SEG(pdta);
    regs.x.dx = FP_OFF(pdta);
    Trace((stderr, "segment:offset of pdta = %x:%x\n", sregs.ds, regs.x.dx));
    Trace((stderr, "&dta = %lx, pdta = %lx\n", (ulg)&dta, (ulg)pdta));
    regs.h.ah = 0x1a;
    intdosx(&regs, &regs, &sregs);
    sregs.ds = FP_SEG(pfcb);
    regs.x.dx = FP_OFF(pfcb);
    pfcb->flag = 0xff;          /* extended FCB */
    pfcb->vattr = 0x08;         /* attribute:  disk volume label */
    pfcb->drive = (uch)nLabelDrive;
#ifdef DEBUG
    Trace((stderr, "segment:offset of pfcb = %x:%x\n", sregs.ds, regs.x.dx));
    Trace((stderr, "&fcb = %lx, pfcb = %lx\n", (ulg)&fcb, (ulg)pfcb));
    Trace((stderr, "(2nd check:  labelling drive %c:)\n", pfcb->drive-1+'A'));
    if (pfcb->flag != fcb.flag)
        fprintf(stderr, "error:  pfcb->flag = %d, fcb.flag = %d\n",
          pfcb->flag, fcb.flag);
    if (pfcb->drive != fcb.drive)
        fprintf(stderr, "error:  pfcb->drive = %d, fcb.drive = %d\n",
          pfcb->drive, fcb.drive);
    if (pfcb->vattr != fcb.vattr)
        fprintf(stderr, "error:  pfcb->vattr = %d, fcb.vattr = %d\n",
          pfcb->vattr, fcb.vattr);
#endif /* DEBUG */
    Trace((stderr, "searching for existing label via FCBs\n"));
    regs.h.ah = 0x11;      /* FCB find first */
#if 0  /* THIS STRNCPY FAILS (MSC bug?): */
    strncpy(pfcb->vn, "???????????", 11);   /* i.e., "*.*" */
    Trace((stderr, "pfcb->vn = %lx\n", (ulg)pfcb->vn));
    Trace((stderr, "flag = %x, drive = %d, vattr = %x, vn = %s = %s.\n",
      fcb.flag, fcb.drive, fcb.vattr, fcb.vn, pfcb->vn));
#endif
    strncpy((char *)fcb.vn, "???????????", 11);   /* i.e., "*.*" */
    Trace((stderr, "fcb.vn = %lx\n", (ulg)fcb.vn));
    Trace((stderr, "regs.h.ah = %x, regs.x.dx = %04x, sregs.ds = %04x\n",
      regs.h.ah, regs.x.dx, sregs.ds));
    Trace((stderr, "flag = %x, drive = %d, vattr = %x, vn = %s = %s.\n",
      fcb.flag, fcb.drive, fcb.vattr, fcb.vn, pfcb->vn));
    intdosx(&regs, &regs, &sregs);
    if (regs.h.al) {
        Trace((stderr, "no label found\n\n"));
        regs.h.ah = 0x16;                 /* FCB create file */
        strncpy((char *)fcb.vn, newlabel, len);
        if (len < 11)   /* fill with spaces */
            strncpy((char *)(fcb.vn+len), "           ", 11-len);
        Trace((stderr, "fcb.vn = %lx  pfcb->vn = %lx\n", (ulg)fcb.vn,
          (ulg)pfcb->vn));
        Trace((stderr, "flag = %x, drive = %d, vattr = %x\n", fcb.flag,
          fcb.drive, fcb.vattr));
        Trace((stderr, "vn = %s = %s.\n", fcb.vn, pfcb->vn));
        intdosx(&regs, &regs, &sregs);
        regs.h.ah = 0x10;                 /* FCB close file */
        if (regs.h.al) {
            Trace((stderr, "unable to write volume name (AL = %x)\n",
              regs.h.al));
            intdosx(&regs, &regs, &sregs);
            return 1;
        } else {
            intdosx(&regs, &regs, &sregs);
            Trace((stderr, "new volume label [%s] written\n", newlabel));
            return 0;
        }
    } else {
        Trace((stderr, "found old label [%s]\n\n", dta.vn));  /* not term. */
        regs.h.ah = 0x17;                 /* FCB rename */
        strncpy((char *)fcb.vn, (char *)dta.vn, 11);
        strncpy((char *)fcb.nn, newlabel, len);
        if (len < 11)                     /* fill with spaces */
            strncpy((char *)(fcb.nn+len), "           ", 11-len);
        Trace((stderr, "fcb.vn = %lx  pfcb->vn = %lx\n", (ulg)fcb.vn,
          (ulg)pfcb->vn));
        Trace((stderr, "fcb.nn = %lx  pfcb->nn = %lx\n", (ulg)fcb.nn,
          (ulg)pfcb->nn));
        Trace((stderr, "flag = %x, drive = %d, vattr = %x\n", fcb.flag,
          fcb.drive, fcb.vattr));
        Trace((stderr, "vn = %s = %s.\n", fcb.vn, pfcb->vn));
        Trace((stderr, "nn = %s = %s.\n", fcb.nn, pfcb->nn));
        intdosx(&regs, &regs, &sregs);
        if (regs.h.al) {
            Trace((stderr, "Unable to change volume name (AL = %x)\n",
              regs.h.al));
            return 1;
        } else {
            Trace((stderr, "volume label changed to [%s]\n", newlabel));
            return 0;
        }
    }
} /* end function volumelabel() */
#endif /* !__GO32__ && !__EMX__ */
void close_outfile()
{
#ifdef __TURBOC__
    union {
        struct ftime ft;        /* system file time record */
        struct {
            ush ztime;          /* date and time words */
            ush zdate;          /* .. same format as in .ZIP file */
        } zt;
    } td;
#endif
#ifdef __TURBOC__
    td.zt.ztime = lrec.last_mod_file_time;
    td.zt.zdate = lrec.last_mod_file_date;
    setftime(fileno(outfile), &td.ft);
#else
    _dos_setftime(fileno(outfile), lrec.last_mod_file_date,
                                   lrec.last_mod_file_time);
#endif
    fclose(outfile);
#ifdef __TURBOC__
    if (_chmod(filename, 1, pInfo->file_attr) != pInfo->file_attr)
        fprintf(stderr, "\nwarning:  file attributes may not be correct\n");
#else
    _dos_setfileattr(filename, pInfo->file_attr);
#endif
} /* end function close_outfile() */
int dateformat()
{
#if (!defined(MSWIN) && !defined(__GO32__) && !defined(__EMX__))
    unsigned short _CountryInfo[18];
    unsigned short far *CountryInfo = _CountryInfo;
    struct SREGS sregs;
    union REGS regs;
    sregs.ds  = FP_SEG(CountryInfo);
    regs.x.dx = FP_OFF(CountryInfo);
    regs.x.ax = 0x3800;
    int86x(0x21, &regs, &regs, &sregs);
    switch(CountryInfo[0]) {
        case 0:
            return DF_MDY;
        case 1:
            return DF_DMY;
        case 2:
            return DF_YMD;
    }
#endif /* !MSWIN  && !__GO32__ && !__EMX__ */
    return DF_MDY;   /* default for systems without locale info */
} /* end function dateformat() */
#if (defined(__GO32__) || defined(__EMX__))
int volatile _doserrno;
void _dos_setftime(int fd, ush dosdate, ush dostime)
{
    asm("movl %0, %%ebx": : "g" (fd));
    asm("movl %0, %%ecx": : "g" (dostime));
    asm("movl %0, %%edx": : "g" (dosdate));
    asm("movl $0x5701, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
}
void _dos_setfileattr(char *name, int attr)
{
    asm("movl %0, %%edx": : "g" (name));
    asm("movl %0, %%ecx": : "g" (attr));
    asm("movl $0x4301, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
}
void _dos_getdrive(unsigned *d)
{
    asm("movl $0x1900, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    asm("xorb %ah, %ah");
    asm("incb %al");
    asm("movl %%eax, %0": "=a" (*d));
}
unsigned _dos_creat(char *path, unsigned attr, int *fd)
{
    asm("movl $0x3c00, %eax");
    asm("movl %0, %%edx": :"g" (path));
    asm("movl %0, %%ecx": :"g" (attr));
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    asm("movl %%eax, %0": "=a" (*fd));
    _doserrno = 0;
    asm("jnc 1f");
    _doserrno = *fd;
    switch (_doserrno) {
    case 3:
           errno = ENOENT;
           break;
    case 4:
           errno = EMFILE;
           break;
    case 5:
           errno = EACCES;
           break;
    }
    asm("1:");
    return _doserrno;
}
unsigned _dos_close(int fd)
{
    asm("movl %0, %%ebx": : "g" (fd));
    asm("movl $0x3e00, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    _doserrno = 0;
    asm("jnc 1f");
    asm ("movl %%eax, %0": "=m" (_doserrno));
    if (_doserrno == 6) {
          errno = EBADF;
    }
    asm("1:");
    return _doserrno;
}
static int volumelabel(char *name)
{
    int fd;
    return _dos_creat(name, FA_LABEL, &fd) ? fd : _dos_close(fd);
}
#endif /* __GO32__ || __EMX__ */
