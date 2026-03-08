/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
static char unkn[16];
static int ratio OF((ulg uc, ulg c));
static void fnprint OF((uch *fn));
#ifndef NO_ZIPINFO  /* strings use up too much space in small-memory systems */
#define UNX_IFMT       0170000     /* Unix file type mask */
#define UNX_IFDIR      0040000     /* Unix directory */
#define UNX_IFREG      0100000     /* Unix regular file */
#define UNX_IFSOCK     0140000     /* Unix socket (BSD, not SysV or Amiga) */
#define UNX_IFLNK      0120000     /* Unix symbolic link (not SysV, Amiga) */
#define UNX_IFBLK      0060000     /* Unix block special       (not Amiga) */
#define UNX_IFCHR      0020000     /* Unix character special   (not Amiga) */
#define UNX_IFIFO      0010000     /* Unix fifo    (BCC, not MSC or Amiga) */
#define UNX_ISUID      04000       /* Unix set user id on execution */
#define UNX_ISGID      02000       /* Unix set group id on execution */
#define UNX_ISVTX      01000       /* Unix directory permissions control */
#define UNX_ENFMT      UNX_ISGID   /* Unix record locking enforcement flag */
#define UNX_IRWXU      00700       /* Unix read, write, execute: owner */
#define UNX_IRUSR      00400       /* Unix read permission: owner */
#define UNX_IWUSR      00200       /* Unix write permission: owner */
#define UNX_IXUSR      00100       /* Unix execute permission: owner */
#define UNX_IRWXG      00070       /* Unix read, write, execute: group */
#define UNX_IRGRP      00040       /* Unix read permission: group */
#define UNX_IWGRP      00020       /* Unix write permission: group */
#define UNX_IXGRP      00010       /* Unix execute permission: group */
#define UNX_IRWXO      00007       /* Unix read, write, execute: other */
#define UNX_IROTH      00004       /* Unix read permission: other */
#define UNX_IWOTH      00002       /* Unix write permission: other */
#define UNX_IXOTH      00001       /* Unix execute permission: other */
#define VMS_IRUSR      UNX_IRUSR   /* VMS read/owner */
#define VMS_IWUSR      UNX_IWUSR   /* VMS write/owner */
#define VMS_IXUSR      UNX_IXUSR   /* VMS execute/owner */
#define VMS_IRGRP      UNX_IRGRP   /* VMS read/group */
#define VMS_IWGRP      UNX_IWGRP   /* VMS write/group */
#define VMS_IXGRP      UNX_IXGRP   /* VMS execute/group */
#define VMS_IROTH      UNX_IROTH   /* VMS read/other */
#define VMS_IWOTH      UNX_IWOTH   /* VMS write/other */
#define VMS_IXOTH      UNX_IXOTH   /* VMS execute/other */
#define AMI_IFMT       06000       /* Amiga file type mask */
#define AMI_IFDIR      04000       /* Amiga directory */
#define AMI_IFREG      02000       /* Amiga regular file */
#define AMI_IHIDDEN    00200       /* to be supported in AmigaDOS 3.x */
#define AMI_ISCRIPT    00100       /* executable script (text command file) */
#define AMI_IPURE      00040       /* allow loading into resident memory */
#define AMI_IARCHIVE   00020       /* not modified since bit was last set */
#define AMI_IREAD      00010       /* can be opened for reading */
#define AMI_IWRITE     00004       /* can be opened for writing */
#define AMI_IEXECUTE   00002       /* executable image, a loadable runfile */
#define AMI_IDELETE    00001       /* can be deleted */
#define LFLAG  3           /* short "ls -l" type listing */
static int   zi_long   OF((void));
static int   zi_short  OF((void));
static char *zi_time   OF((ush *datez, ush *timez));
int zi_opts(pargc, pargv)
    int *pargc;
    char ***pargv;
{
    char   **argv, *s;
    int    argc, c, error=FALSE, negative=0;
    int    hflag_slmv=TRUE, hflag_2=FALSE;  /* diff options => diff defaults */
    int    tflag_slm=TRUE, tflag_2v=FALSE;
    int    explicit_h=FALSE, explicit_t=FALSE;
#ifdef MACOS
    lflag = LFLAG;   /* reset default on each call */
#endif
    argc = *pargc;
    argv = *pargv;
    while (--argc > 0 && (*++argv)[0] == '-') {
        s = argv[0] + 1;
        while ((c = *s++) != 0) {    /* "!= 0":  prevent Turbo C warning */
            switch (c) {
                case '-':
                    ++negative;
                    break;
                case '1':      /* shortest listing:  JUST filenames */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 1;
                    break;
                case '2':      /* just filenames, plus headers if specified */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 2;
                    break;
                case 'h':      /* header line */
                    if (negative)
                        hflag_2 = hflag_slmv = FALSE, negative = 0;
                    else {
                        hflag_2 = hflag_slmv = explicit_h = TRUE;
                        if (lflag == -1)
                            lflag = 0;
                    }
                    break;
                case 'l':      /* longer form of "ls -l" type listing */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 5;
                    break;
                case 'm':      /* medium form of "ls -l" type listing */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 4;
                    break;
                case 's':      /* default:  shorter "ls -l" type listing */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 3;
                    break;
                case 't':      /* totals line */
                    if (negative)
                        tflag_2v = tflag_slm = FALSE, negative = 0;
                    else {
                        tflag_2v = tflag_slm = explicit_t = TRUE;
                        if (lflag == -1)
                            lflag = 0;
                    }
                    break;
                case 'v':      /* turbo-verbose listing */
                    if (negative)
                        lflag = -2, negative = 0;
                    else
                        lflag = 10;
                    break;
                case 'Z':      /* ZipInfo mode:  ignore */
                    break;
                case 'z':      /* print zipfile comment */
                    if (negative)
                        zflag = negative = 0;
                    else
                        zflag = 1;
                    break;
                default:
                    error = TRUE;
                    break;
            }
        }
    }
    if ((argc-- == 0) || error) {
        *pargc = argc;
        *pargv = argv;
        return usage(error);
    }
    if ((lflag < 0) || ((argc > 0) && (lflag == 0)))
        lflag = LFLAG;
    switch (lflag) {
        case 0:   /* 0:  can only occur if either -t or -h explicitly given; */
        case 2:   /*  therefore set both flags equal to normally false value */
            hflag = hflag_2;
            tflag = tflag_2v;
            break;
        case 1:   /* only filenames, *always* */
            hflag = FALSE;
            tflag = FALSE;
            zflag = FALSE;
            break;
        case 3:
        case 4:
        case 5:
            hflag = ((argc > 0) && !explicit_h)? FALSE : hflag_slmv;
            tflag = ((argc > 0) && !explicit_t)? FALSE : tflag_slm;
            break;
        case 10:
            hflag = hflag_slmv;
            tflag = tflag_2v;
            break;
    }
    *pargc = argc;
    *pargv = argv;
    return 0;
} /* end function zi_opts() */
int zi_end_central()   /* return PK-type error code */
{
    int  error = PK_COOL;
    if (hflag)
        printf(((int)strlen(zipfn) < 39)?
          "Archive:  %s   %ld bytes   %d file%s\n" :
          "Archive:  %s   %ld   %d\n", zipfn, (long)ziplen,
          ecrec.total_entries_central_dir,
          (ecrec.total_entries_central_dir==1)? "":"s");
    if (lflag > 9) {
        printf("\nEnd-of-central-directory record:\n");
        printf("-------------------------------\n\n");
        printf("\
  Actual offset of end-of-central-dir record:   %9ld (%.8lXh)\n\
  Expected offset of end-of-central-dir record: %9ld (%.8lXh)\n\
  (based on the length of the central directory and its expected offset)\n\n",
          (long)real_ecrec_offset, (long)real_ecrec_offset,
          (long)expect_ecrec_offset, (long)expect_ecrec_offset);
        if (ecrec.number_this_disk == 0) {
            printf("\
  This zipfile constitutes the sole disk of a single-part archive; its\n\
  central directory contains %u %s.  The central directory is %lu\n\
  (%.8lXh) bytes long, and its (expected) offset in bytes from the\n\
  beginning of the zipfile is %lu (%.8lXh).\n\n",
              ecrec.total_entries_central_dir,
              (ecrec.total_entries_central_dir == 1)? "entry" : "entries",
              ecrec.size_central_directory, ecrec.size_central_directory,
              ecrec.offset_start_central_directory,
              ecrec.offset_start_central_directory);
        } else {
            printf("\
  This zipfile constitutes disk %u of a multi-part archive.  The central\n\
  directory starts on disk %u; %u of its entries %s contained within\n\
  this zipfile, out of a total of %u %s.  The entire central\n\
  directory is %lu (%.8lXh) bytes long, and its offset in bytes from\n\
  the beginning of the zipfile in which it begins is %lu (%.8lXh).\n\n",
              ecrec.number_this_disk,
              ecrec.num_disk_with_start_central_dir,
              ecrec.num_entries_centrl_dir_ths_disk,
              (ecrec.num_entries_centrl_dir_ths_disk == 1)? "is" : "are",
              ecrec.total_entries_central_dir,
              (ecrec.total_entries_central_dir == 1) ? "entry" : "entries",
              ecrec.size_central_directory, ecrec.size_central_directory,
              ecrec.offset_start_central_directory,
              ecrec.offset_start_central_directory);
        }
        if (!ecrec.zipfile_comment_length)
            printf("  There is no zipfile comment.\n");
        else {
            printf("  The zipfile comment is %u bytes long and contains the following text:\n\n",
              ecrec.zipfile_comment_length );
            printf("======================== zipfile comment begins ==========================\n");
            if (do_string(ecrec.zipfile_comment_length, DISPLAY))
                error = PK_WARN;
            printf("========================= zipfile comment ends ===========================\n");
            if (error)
                printf("\n  The zipfile comment is truncated.\n");
        } /* endif (comment exists) */
    } else if (zflag && ecrec.zipfile_comment_length) {
        if (do_string(ecrec.zipfile_comment_length,DISPLAY)) {
            fprintf(stderr, "\ncaution:  zipfile comment truncated\n");
            error = PK_WARN;
        }
    } /* endif (verbose) */
    return error;
} /* end function zi_end_central() */
int zipinfo()   /* return PK-type error code */
{
    int   j, do_this_file=FALSE, error, error_in_archive=PK_COOL;
    int   *fn_matched=NULL, *xn_matched=NULL;
    ush   members=0;
    ulg   tot_csize=0L, tot_ucsize=0L;
    if (filespecs > 0  &&
        (fn_matched=(int *)malloc(filespecs*sizeof(int))) != NULL)
        for (j = 0;  j < filespecs;  ++j)
            fn_matched[j] = FALSE;
    if (xfilespecs > 0  &&
        (xn_matched=(int *)malloc(xfilespecs*sizeof(int))) != NULL)
        for (j = 0;  j < xfilespecs;  ++j)
            xn_matched[j] = FALSE;
    pInfo->lcflag = 0;    /* used in do_string():  never TRUE in zipinfo mode */
    pInfo->textmode = 0;  /* so one can read on screen (but is it ever used?) */
    for (j = 0;  j < (int)ecrec.total_entries_central_dir;  ++j) {
        if (readbuf(sig, 4) <= 0)
            return PK_EOF;
        if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
            fprintf(stderr, CentSigMsg, j);  /* sig not found */
            return PK_BADERR;
        }
        if ((error = get_cdir_file_hdr()) != PK_COOL)
            return error;       /* only PK_EOF defined */
        if (IS_VOLID(crec.external_file_attributes) &&
            (pInfo->hostnum == FS_FAT_ || pInfo->hostnum == FS_HPFS_ ||
             pInfo->hostnum == FS_NTFS_ || pInfo->hostnum == ATARI_))
            pInfo->vollabel = TRUE;
        else
            pInfo->vollabel = FALSE;
        if ((error = do_string(crec.filename_length, FILENAME)) != PK_COOL) {
          error_in_archive = error;   /* might be warning */
          if (error > PK_WARN)        /* fatal */
              return error;
        }
        if (!process_all_files) {    /* check if specified on command line */
            char  **pfn = pfnames-1;
            do_this_file = FALSE;
            while (*++pfn)
                if (match(filename, *pfn, 0)) {
                    do_this_file = TRUE;
                    if (fn_matched)
                        fn_matched[pfn-pfnames] = TRUE;
                    break;       /* found match, so stop looping */
                }
            if (do_this_file) {  /* check if this is an excluded file */
                char  **pxn = pxnames-1;
                while (*++pxn)
                    if (match(filename, *pxn, 0)) {
                        do_this_file = FALSE;
                        if (xn_matched)
                            xn_matched[pxn-pxnames] = TRUE;
                        break;
                    }
            }
        }
        if (process_all_files || do_this_file) {
            switch (lflag) {
                case 1:
                case 2:
                    fnprint((uch *)filename);
                    SKIP_(crec.extra_field_length)
                    SKIP_(crec.file_comment_length)
                    break;
                case 3:
                case 4:
                case 5:
                    if ((error = zi_short()) != PK_COOL) {
                        error_in_archive = error;   /* might be warning */
                        if (error > PK_WARN)        /* fatal */
                            return error;
                    }
                    break;
                case 10:
#ifdef T20_VMS  /* GRR:  add cbreak-style "more" */
                    printf("\nCentral directory entry #%d:\n", j);
#else
                    printf("%s\nCentral directory entry #%d:\n", "\014", j);
#endif
                    printf("---------------------------\n\n");
                    if ((error = zi_long()) != PK_COOL) {
                      error_in_archive = error;   /* might be warning */
                      if (error > PK_WARN)        /* fatal */
                          return error;
                    }
                    break;
                default:
                    SKIP_(crec.extra_field_length)
                    SKIP_(crec.file_comment_length)
                    break;
            } /* end switch (lflag) */
            tot_csize += crec.csize;
            tot_ucsize += crec.ucsize;
            if (crec.general_purpose_bit_flag & 1)
                tot_csize -= 12;   /* don't count encryption header */
            ++members;
        } else {   /* not listing */
            SKIP_(crec.extra_field_length)
            SKIP_(crec.file_comment_length)
        } /* end if (list member?) */
    } /* end for-loop (j: member files) */
    if (readbuf(sig, 4) <= 0)  /* disk error? */
        return PK_EOF;
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
        error_in_archive = PK_WARN;
    }
    if (tflag) {
        char *sgn = "";
        int cfactor = ratio(tot_ucsize, tot_csize);
        if (cfactor < 0) {
            sgn = "-";
            cfactor = -cfactor;
        }
        printf(
        "%d file%s, %lu bytes uncompressed, %lu bytes compressed:  %s%d.%d%%\n",
          members, (members==1)? "":"s", tot_ucsize, tot_csize, sgn, cfactor/10,
          cfactor%10);
    }
    if (fn_matched) {
        for (j = 0;  j < filespecs;  ++j)
            if (!fn_matched[j])
                fprintf(stderr, "caution: filename not matched:  %s\n",
                  pfnames[j]);
        free(fn_matched);
    }
    if (xn_matched) {
        for (j = 0;  j < xfilespecs;  ++j)
            if (!xn_matched[j])
                fprintf(stderr, "caution: excluded filename not matched:  %s\n",
                  pxnames[j]);
        free(xn_matched);
    }
    return error_in_archive;
} /* end function zipinfo() */
static int zi_long()   /* return PK-type error code */
{
    int          error, error_in_archive=PK_COOL;
    ush          hostnum, hostver, extnum, extver, methnum, xattr;
    char         workspace[12], attribs[22];
    static char  *os[NUM_HOSTS+1] = {"MS-DOS, OS/2 or NT FAT", "Amiga",
                     "VAX VMS", "Unix", "VM/CMS", "Atari ST", "OS/2 or NT HPFS",
                     "Macintosh", "Z-System", "CP/M", "TOPS-20", "NT NTFS",
                     "unknown" };
    static char  *method[NUM_METHODS+1] = {"none (stored)", "shrunk",
                     "reduced (factor 1)", "reduced (factor 2)",
                     "reduced (factor 3)", "reduced (factor 4)",
                     "imploded", "tokenized", "deflated", unkn};
    static char  *dtype[4] = {"normal", "maximum", "fast", "superfast"};
    hostnum = MIN(crec.version_made_by[1], NUM_HOSTS);
    hostver = crec.version_made_by[0];
    extnum = MIN(crec.version_needed_to_extract[1], NUM_HOSTS);
    extver = crec.version_needed_to_extract[0];
    methnum = MIN(crec.compression_method, NUM_METHODS);
    if (methnum == NUM_METHODS)
        sprintf(unkn, "unknown (%d)", crec.compression_method);
    putchar(' ');  putchar(' ');  fnprint((uch *)filename);
    printf("\n  host operating system (created on):               %s\n",
      os[hostnum]);
    printf("  version of encoding software:                     %d.%d\n",
      hostver/10, hostver%10);
    printf("  minimum operating system compatibility required:  %s\n",
      os[extnum]);
    printf("  minimum software version required to extract:     %d.%d\n",
      extver/10, extver%10);
    printf("  compression method:                               %s\n",
      method[methnum]);
    if (methnum == IMPLODED) {
        printf("  size of sliding dictionary (implosion):           %cK\n",
          (crec.general_purpose_bit_flag & 2)? '8' : '4');
        printf("  number of Shannon-Fano trees (implosion):         %c\n",
          (crec.general_purpose_bit_flag & 4)? '3' : '2');
    } else if (methnum == DEFLATED) {
        ush  dnum=(crec.general_purpose_bit_flag>>1) & 3;
        printf("  compression sub-type (deflation):                 %s\n",
          dtype[dnum]);
    }
    printf("  file security status:                             %sencrypted\n",
      (crec.general_purpose_bit_flag & 1)? "" : "not ");
    printf("  extended local header:                            %s\n",
      (crec.general_purpose_bit_flag & 8)? "yes" : "no");
    printf("  file last modified on:                            %s\n",
      zi_time(&crec.last_mod_file_date, &crec.last_mod_file_time));
    printf("  32-bit CRC value (hex):                           %.8lx\n",
      crec.crc32);
    printf("  compressed size:                                  %lu bytes\n",
      crec.csize);
    printf("  uncompressed size:                                %lu bytes\n",
      crec.ucsize);
    printf("  length of filename:                               %u characters\n",
      crec.filename_length);
    printf("  length of extra field:                            %u bytes\n",
      crec.extra_field_length);
    printf("  length of file comment:                           %u characters\n",
      crec.file_comment_length);
    printf("  disk number on which file begins:                 disk %u\n",
      crec.disk_number_start);
    printf("  apparent file type:                               %s\n",
      (crec.internal_file_attributes & 1)? "text" : "binary");
    xattr = (ush)((crec.external_file_attributes >> 16) & 0xFFFF);
    if (hostnum == VMS_) {
        char   *p=attribs, *q=attribs+1;
        int    i, j, k;
        for (k = 0;  k < 12;  ++k)
            workspace[k] = 0;
        if (xattr & VMS_IRUSR)
            workspace[0] = 'R';
        if (xattr & VMS_IWUSR) {
            workspace[1] = 'W';
            workspace[3] = 'D';
        }
        if (xattr & VMS_IXUSR)
            workspace[2] = 'E';
        if (xattr & VMS_IRGRP)
            workspace[4] = 'R';
        if (xattr & VMS_IWGRP) {
            workspace[5] = 'W';
            workspace[7] = 'D';
        }
        if (xattr & VMS_IXGRP)
            workspace[6] = 'E';
        if (xattr & VMS_IROTH)
            workspace[8] = 'R';
        if (xattr & VMS_IWOTH) {
            workspace[9] = 'W';
            workspace[11] = 'D';
        }
        if (xattr & VMS_IXOTH)
            workspace[10] = 'E';
        *p++ = '(';
        for (k = j = 0;  j < 3;  ++j) {    /* loop over groups of permissions */
            for (i = 0;  i < 4;  ++i, ++k)  /* loop over perms within a group */
                if (workspace[k])
                    *p++ = workspace[k];
            *p++ = ',';                       /* group separator */
            if (j == 0)
                while ((*p++ = *q++) != ','); /* system, owner perms are same */
        }
        *p-- = 0;
        *p = ')';   /* overwrite last comma */
        printf("  VMS file attributes (%06o octal):               %s\n",
          xattr, attribs);
    } else if (hostnum == AMIGA_) {
        switch (xattr & AMI_IFMT) {
            case AMI_IFDIR:  attribs[0] = 'd';  break;
            case AMI_IFREG:  attribs[0] = '-';  break;
            default:         attribs[0] = '?';  break;
        }
        attribs[1] = (xattr & AMI_IHIDDEN)?   'h' : '-';
        attribs[2] = (xattr & AMI_ISCRIPT)?   's' : '-';
        attribs[3] = (xattr & AMI_IPURE)?     'p' : '-';
        attribs[4] = (xattr & AMI_IARCHIVE)?  'a' : '-';
        attribs[5] = (xattr & AMI_IREAD)?     'r' : '-';
        attribs[6] = (xattr & AMI_IWRITE)?    'w' : '-';
        attribs[7] = (xattr & AMI_IEXECUTE)?  'e' : '-';
        attribs[8] = (xattr & AMI_IDELETE)?   'd' : '-';
        attribs[9] = 0;   /* better dlm the string */
        printf("  Amiga file attributes (%06o octal):             %s\n",
          xattr, attribs);
    } else if ((hostnum != FS_FAT_) && (hostnum != FS_HPFS_) &&
        (hostnum != FS_NTFS_)) {
        switch (xattr & UNX_IFMT) {
            case UNX_IFDIR:   attribs[0] = 'd';  break;
            case UNX_IFREG:   attribs[0] = '-';  break;
            case UNX_IFLNK:   attribs[0] = 'l';  break;
            case UNX_IFBLK:   attribs[0] = 'b';  break;
            case UNX_IFCHR:   attribs[0] = 'c';  break;
            case UNX_IFIFO:   attribs[0] = 'p';  break;
            case UNX_IFSOCK:  attribs[0] = 's';  break;
            default:          attribs[0] = '?';  break;
        }
        attribs[1] = (xattr & UNX_IRUSR)? 'r' : '-';
        attribs[4] = (xattr & UNX_IRGRP)? 'r' : '-';
        attribs[7] = (xattr & UNX_IROTH)? 'r' : '-';
        attribs[2] = (xattr & UNX_IWUSR)? 'w' : '-';
        attribs[5] = (xattr & UNX_IWGRP)? 'w' : '-';
        attribs[8] = (xattr & UNX_IWOTH)? 'w' : '-';
        if (xattr & UNX_IXUSR)
            attribs[3] = (xattr & UNX_ISUID)? 's' : 'x';
        else
            attribs[3] = (xattr & UNX_ISUID)? 'S' : '-';   /* S = undefined */
        if (xattr & UNX_IXGRP)
            attribs[6] = (xattr & UNX_ISGID)? 's' : 'x';   /* == UNX_ENFMT */
        else
            attribs[6] = (xattr & UNX_ISGID)? 'l' : '-';
        if (xattr & UNX_IXOTH)
            attribs[9] = (xattr & UNX_ISVTX)? 't' : 'x';   /* "sticky bit" */
        else
            attribs[9] = (xattr & UNX_ISVTX)? 'T' : '-';   /* T = undefined */
        attribs[10] = 0;
        printf("  Unix file attributes (%06o octal):              %s\n",
          xattr, attribs);
    } else {
        printf("  non-MSDOS external file attributes:               %06lX hex\n"
          , crec.external_file_attributes >> 8);
    } /* endif (hostnum: external attributes format) */
    if ((xattr=(ush)(crec.external_file_attributes & 0xFF)) == 0)
        printf("  MS-DOS file attributes (%02X hex):                  none\n",
          xattr);
    else if (xattr == 1)
        printf(
          "  MS-DOS file attributes (%02X hex):                  read-only\n",
          xattr);
    else
        printf(
         "  MS-DOS file attributes (%02X hex):                  %s%s%s%s%s%s\n",
          xattr, (xattr&1)?"rdo ":"", (xattr&2)?"hid ":"", (xattr&4)?"sys ":"",
          (xattr&8)?"lab ":"", (xattr&16)?"dir ":"", (xattr&32)?"arc":"");
    printf(
     "  offset of local header from start of archive:     %lu (%.8lXh) bytes\n",
      crec.relative_offset_local_header, crec.relative_offset_local_header);
    if (crec.extra_field_length > 0) {
#if TRUE
        ulg ea_size;
        if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0) {
            error_in_archive = error;
            if (error > PK_WARN)   /* fatal:  can't continue */
                return error;
        }
        if ((ea_size = SizeOfEAs(extra_field)) != 0)
            printf("\n\
  This file has %lu bytes of OS/2 EA's in the local extra field.\n\
  (May not match OS/2 \"dir\" amount due to storage method.)\n\n",
              ea_size);
        else
            printf("\n  There is an unknown extra field (skipping).\n");
#else
        printf("\n  There is an extra field (skipping).\n");
        SKIP_(crec.extra_field_length)
#endif
    } else
        printf("\n");
    if (!crec.file_comment_length)
        printf("  There is no file comment.\n");
    else {
        printf("\
------------------------- file comment begins ----------------------------\n");
        if ((error = do_string(crec.file_comment_length, DISPLAY)) != PK_COOL) {
          error_in_archive = error;   /* might be warning */
          if (error > PK_WARN)   /* fatal */
              return error;
        }
        printf("\
-------------------------- file comment ends -----------------------------\n");
    }
    return error_in_archive;
} /* end function zi_long() */
static int zi_short()   /* return PK-type error code */
{
    int           k, error, error_in_archive=PK_COOL;
    ush           methnum, hostnum, hostver, xattr;
    char          *p, workspace[12], attribs[16];
    static char   impl[5]="i#:#", defl[5]="def#";
    static char   dtype[5]="NXFS";  /* normal, maximum, fast, superfast */
    static char   *os[NUM_HOSTS+1] = {"fat", "ami", "vms", "unx", "cms",
                      "atr", "hpf", "mac", "zzz", "cpm", "t20", "ntf", "???" };
    static char   *method[NUM_METHODS+1] = {"stor", "shrk", "re:1", "re:2",
                      "re:3", "re:4", impl, "tokn", defl, unkn};
    methnum = MIN(crec.compression_method, NUM_METHODS);
    hostnum = MIN(crec.version_made_by[1], NUM_HOSTS);
    hostver = crec.version_made_by[0];
    if (methnum == IMPLODED) {
        impl[1] = (char) ((crec.general_purpose_bit_flag & 2)? '8' : '4');
        impl[3] = (char) ((crec.general_purpose_bit_flag & 4)? '3' : '2');
    } else if (methnum == DEFLATED) {
        ush  dnum=(crec.general_purpose_bit_flag>>1) & 3;
        defl[3] = dtype[dnum];
    } else if (methnum == NUM_METHODS) {   /* unknown */
        sprintf(unkn, "u%03d", crec.compression_method);
    }
    for (k = 0;  k < 15;  ++k)
        attribs[k] = ' ';
    attribs[15] = 0;
    xattr = (ush)((crec.external_file_attributes >> 16) & 0xFFFF);
    switch (hostnum) {
        case VMS_:
            {   int    i, j;
                for (k = 0;  k < 12;  ++k)
                    workspace[k] = 0;
                if (xattr & VMS_IRUSR)
                    workspace[0] = 'R';
                if (xattr & VMS_IWUSR) {
                    workspace[1] = 'W';
                    workspace[3] = 'D';
                }
                if (xattr & VMS_IXUSR)
                    workspace[2] = 'E';
                if (xattr & VMS_IRGRP)
                    workspace[4] = 'R';
                if (xattr & VMS_IWGRP) {
                    workspace[5] = 'W';
                    workspace[7] = 'D';
                }
                if (xattr & VMS_IXGRP)
                  workspace[6] = 'E';
                if (xattr & VMS_IROTH)
                    workspace[8] = 'R';
                if (xattr & VMS_IWOTH) {
                    workspace[9] = 'W';
                    workspace[11] = 'D';
                }
                if (xattr & VMS_IXOTH)
                    workspace[10] = 'E';
                p = attribs;
                for (k = j = 0;  j < 3;  ++j) {     /* groups of permissions */
                    for (i = 0;  i < 4;  ++i, ++k)  /* perms within a group */
                        if (workspace[k])
                            *p++ = workspace[k];
                    *p++ = ',';                     /* group separator */
                }
                *--p = ' ';   /* overwrite last comma */
                if ((p - attribs) < 12)
                    sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
            }
            break;
        case FS_FAT_:
        case FS_HPFS_:
        case FS_NTFS_:
            xattr = (ush)(crec.external_file_attributes & 0xFF);
            sprintf(attribs, ".r.-...     %d.%d", hostver/10, hostver%10);
            attribs[2] = (xattr & 0x01)? '-' : 'w';
            attribs[5] = (xattr & 0x02)? 'h' : '-';
            attribs[6] = (xattr & 0x04)? 's' : '-';
            attribs[0] = (xattr & 0x10)? 'd' : '-';
            attribs[4] = (xattr & 0x20)? 'a' : '-';
            if (IS_VOLID(xattr))
                attribs[0] = 'V';
            else if ((p = strrchr(filename, '.')) != NULL) {
                ++p;
                if (STRNICMP(p, "com", 3) == 0 || STRNICMP(p, "exe", 3) == 0 ||
                    STRNICMP(p, "btm", 3) == 0 || STRNICMP(p, "cmd", 3) == 0 ||
                    STRNICMP(p, "bat", 3) == 0)
                    attribs[3] = 'x';
            }
            break;
        case AMIGA_:
            switch (xattr & AMI_IFMT) {
                case AMI_IFDIR:  attribs[0] = 'd';  break;
                case AMI_IFREG:  attribs[0] = '-';  break;
                default:         attribs[0] = '?';  break;
            }
            attribs[1] = (xattr & AMI_IHIDDEN)?   'h' : '-';
            attribs[2] = (xattr & AMI_ISCRIPT)?   's' : '-';
            attribs[3] = (xattr & AMI_IPURE)?     'p' : '-';
            attribs[4] = (xattr & AMI_IARCHIVE)?  'a' : '-';
            attribs[5] = (xattr & AMI_IREAD)?     'r' : '-';
            attribs[6] = (xattr & AMI_IWRITE)?    'w' : '-';
            attribs[7] = (xattr & AMI_IEXECUTE)?  'e' : '-';
            attribs[8] = (xattr & AMI_IDELETE)?   'd' : '-';
            sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
            break;
        default:   /* assume Unix-like */
            switch (xattr & UNX_IFMT) {
                case UNX_IFDIR:   attribs[0] = 'd';  break;
                case UNX_IFREG:   attribs[0] = '-';  break;
                case UNX_IFLNK:   attribs[0] = 'l';  break;
                case UNX_IFBLK:   attribs[0] = 'b';  break;
                case UNX_IFCHR:   attribs[0] = 'c';  break;
                case UNX_IFIFO:   attribs[0] = 'p';  break;
                case UNX_IFSOCK:  attribs[0] = 's';  break;
                default:          attribs[0] = '?';  break;
            }
            attribs[1] = (xattr & UNX_IRUSR)? 'r' : '-';
            attribs[4] = (xattr & UNX_IRGRP)? 'r' : '-';
            attribs[7] = (xattr & UNX_IROTH)? 'r' : '-';
            attribs[2] = (xattr & UNX_IWUSR)? 'w' : '-';
            attribs[5] = (xattr & UNX_IWGRP)? 'w' : '-';
            attribs[8] = (xattr & UNX_IWOTH)? 'w' : '-';
            if (xattr & UNX_IXUSR)
                attribs[3] = (xattr & UNX_ISUID)? 's' : 'x';
            else
                attribs[3] = (xattr & UNX_ISUID)? 'S' : '-';  /* S==undefined */
            if (xattr & UNX_IXGRP)
                attribs[6] = (xattr & UNX_ISGID)? 's' : 'x';  /* == UNX_ENFMT */
            else
                attribs[6] = (xattr & UNX_ISGID)? 'S' : '-';  /* SunOS 4.1.x */
            if (xattr & UNX_IXOTH)
                attribs[9] = (xattr & UNX_ISVTX)? 't' : 'x';  /* "sticky bit" */
            else
                attribs[9] = (xattr & UNX_ISVTX)? 'T' : '-';  /* T==undefined */
            sprintf(&attribs[12], "%d.%d", hostver/10, hostver%10);
            break;
    } /* end switch (hostnum: external attributes format) */
    printf("%s %s %7lu %c%c", attribs, os[hostnum], crec.ucsize,
      (crec.general_purpose_bit_flag & 1)?
      ((crec.internal_file_attributes & 1)? 'T' : 'B') :   /* encrypted */
      ((crec.internal_file_attributes & 1)? 't' : 'b'),    /* plaintext */
      (crec.general_purpose_bit_flag & 8)? (crec.extra_field_length? 'X' : 'l')
                                        : (crec.extra_field_length? 'x' : '-'));
    if (lflag == 4) {
        ulg csiz = crec.csize;
        if (crec.general_purpose_bit_flag & 1)
            csiz -= 12;    /* if encrypted, don't count encryption header */
        printf("%3d%%", (ratio(crec.ucsize,csiz) + 5)/10);
    } else if (lflag == 5)
        printf(" %7lu", crec.csize);
    printf(" %s %s ", method[methnum],
      zi_time(&crec.last_mod_file_date, &crec.last_mod_file_time));
    fnprint((uch *)filename);
    SKIP_(crec.extra_field_length)
    SKIP_(crec.file_comment_length)
    return error_in_archive;
} /* end function zi_short() */
static char *zi_time(datez, timez)
    ush *datez, *timez;
{
    ush          yr, mo, dy, hh, mm, ss;
    static char  d_t_str[21];
    static char  *month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    yr = ((*datez >> 9) & 0x7f) + 80;
    mo = ((*datez >> 5) & 0x0f) - 1;
    dy = *datez & 0x1f;
    hh = (*timez >> 11) & 0x1f;
    mm = (*timez >> 5) & 0x3f;
    ss = (*timez & 0x1f) * 2;
    if ((lflag >= 3) && (lflag <= 5))
        sprintf(d_t_str, "%2u-%s-%u %02u:%02u", dy, month[mo], yr, hh, mm);
    else if (lflag > 9)  /* verbose listing format */
        sprintf(d_t_str, "%u %s %u %02u:%02u:%02u", yr+1900, month[mo], dy,
          hh, mm, ss);
    return d_t_str;
} /* end function zi_time() */
#endif /* !NO_ZIPINFO */
#if (!defined(NO_ZIPINFO) || defined(OS2))
#define EAID   0x0009      /* OS/2 extended-attributes extra-field ID, for */
typedef struct {           /*  OS/2 zipfile info in both OS/2 and non-OS2 */
    unsigned short nID;    /*  environments */
    unsigned short nSize;
    ulg lSize;             /* GRR:  need to do same for VMS, Mac, etc. */
} EAHEADER, *PEAHEADER;
ulg SizeOfEAs(extra_field)
    void   *extra_field;
{
    EAHEADER *pEAblock = (PEAHEADER)extra_field;
    if (extra_field != NULL  &&  pEAblock->nID == EAID)
        return pEAblock->lSize;
    return 0L;
}
#endif /* !NO_ZIPINFO || OS2 */
char *Headers[][2] = {
    {" Length    Date    Time    Name",
     " ------    ----    ----    ----"},
    {" Length  Method   Size  Ratio   Date    Time   CRC-32     Name",
     " ------  ------   ----  -----   ----    ----   ------     ----"}
};
int list_files()    /* return PK-type error code */
{
    char sgn, cfactorstr[10];
    int do_this_file=FALSE, cfactor, error, error_in_archive=PK_COOL;
    int longhdr=(vflag>1), date_format, methnum;
    ush j, yr, mo, dy, hh, mm, members=0;
    ulg csiz, tot_csize=0L, tot_ucsize=0L;
#ifdef OS2
    ulg ea_size, tot_easize=0L, tot_eafiles=0L;
#endif
#ifdef MSWIN
    PSTR psLBEntry;  /* list box entry */
#endif
    min_info info;
    static char defl[]="Defl:#", dtype[]="NXFS";   /* see zi_short() */
    static char *method[NUM_METHODS+1] =
        {"Stored", "Shrunk", "Reduce1", "Reduce2", "Reduce3", "Reduce4",
         "Implode", "Token", defl, unkn};
    pInfo = &info;
    date_format = DATE_FORMAT;
#ifndef MSWIN
    if (qflag < 2)
        if (U_flag)
            printf("%s\n%s\n", Headers[longhdr][0], Headers[longhdr][1]);
        else
            printf("%s (\"^\" ==> case\n%s   conversion)\n",
              Headers[longhdr][0], Headers[longhdr][1]);
#endif /* !MSWIN */
    for (j = 0; j < ecrec.total_entries_central_dir; ++j) {
        if (readbuf(sig, 4) <= 0)
            return PK_EOF;
        if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
            fprintf(stderr, CentSigMsg, j);  /* sig not found */
            fprintf(stderr, ReportMsg);   /* check binary transfers */
            return PK_BADERR;
        }
        if ((error = process_cdir_file_hdr()) != PK_COOL)
            return error;       /* only PK_EOF defined */
        if ((error = do_string(crec.filename_length, FILENAME)) != PK_COOL) {
            error_in_archive = error;             /*  ^--(uses pInfo->lcflag) */
            if (error > PK_WARN)   /* fatal:  can't continue */
                return error;
        }
        if (extra_field != (uch *)NULL) {
            free(extra_field);
            extra_field = (uch *)NULL;
        }
        if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0) {
            error_in_archive = error;
            if (error > PK_WARN)      /* fatal */
                return error;
        }
        if (!process_all_files) {   /* check if specified on command line */
            char **pfn = pfnames-1;
            do_this_file = FALSE;
            while (*++pfn)
                if (match(filename, *pfn, pInfo->lcflag)) {
                    do_this_file = TRUE;
                    break;       /* found match, so stop looping */
                }
            if (do_this_file) {  /* check if this is an excluded file */
                char **pxn = pxnames-1;
                while (*++pxn)
                    if (match(filename, *pxn, pInfo->lcflag)) {
                        do_this_file = FALSE;
                        break;
                    }
            }
        }
        if (process_all_files || do_this_file) {
            yr = (((crec.last_mod_file_date >> 9) & 0x7f) + 80) % (unsigned)100;
            mo = (crec.last_mod_file_date >> 5) & 0x0f;
            dy = crec.last_mod_file_date & 0x1f;
            switch (date_format) {
                case DF_YMD:
                    hh = mo; mo = yr; yr = dy; dy = hh;
                    break;
                case DF_DMY:
                    hh = mo; mo = dy; dy = hh;
            }
            hh = (crec.last_mod_file_time >> 11) & 0x1f;
            mm = (crec.last_mod_file_time >> 5) & 0x3f;
            csiz = crec.csize;
            if (crec.general_purpose_bit_flag & 1)
                csiz -= 12;   /* if encrypted, don't count encryption header */
            if ((cfactor = ratio(crec.ucsize, csiz)) < 0) {
                sgn = '-';
                cfactor = (-cfactor + 5) / 10;
            } else {
                sgn = ' ';
                cfactor = (cfactor + 5) / 10;
            }
            sprintf(cfactorstr, "%c%d%%", sgn, cfactor);
            methnum = MIN(crec.compression_method, NUM_METHODS);
            if (methnum == DEFLATED)
                defl[5] = dtype[(crec.general_purpose_bit_flag>>1) & 3];
            else if (methnum == NUM_METHODS)
                sprintf(unkn, "Unk:%03d", crec.compression_method);
#if 0       /* GRR/Euro:  add this? */
#if defined(DOS_NT_OS2) || defined(UNIX)
            for (p = filename;  *p;  ++p)
                if (!isprint(*p))
                    *p = '?';  /* change non-printable chars to '?' */
#endif /* DOS_NT_OS2 || UNIX */
#endif /* 0 */
#ifdef MSWIN
#ifdef NEED_EARLY_REDRAW
            if (j == (ecrec.total_entries_central_dir-1))
                (void)SendMessage(hWndList, WM_SETREDRAW, TRUE, 0L);
#endif /* NEED_EARLY_REDRAW */
            psLBEntry =
              (PSTR)LocalAlloc(LMEM_FIXED, FILNAMSIZ+LONG_FORM_FNAME_INX);
            OemToAnsi(filename, filename);  /* translate to ANSI */
            if (longhdr) {
                wsprintf(psLBEntry,
                 "%7lu  %-7s%7lu %4s  %02u-%02u-%02u  %02u:%02u  %08lx  %c%s",
                  crec.ucsize, (LPSTR)method[methnum], csiz, cfactorstr,
                  mo, dy, yr, hh, mm, crec.crc32, (pInfo->lcflag?'^':' '),
                  (LPSTR)filename);
                SendMessage(hWndList, LB_ADDSTRING, 0,
                  (LONG)(LPSTR)psLBEntry);
            } else {
                wsprintf(psLBEntry, "%7lu  %02u-%02u-%02u  %02u:%02u  %c%s",
                  crec.ucsize, mo, dy, yr, hh, mm, (pInfo->lcflag?'^':' '),
                  (LPSTR)filename);
                SendMessage(hWndList, LB_ADDSTRING, 0,
                  (LONG)(LPSTR)psLBEntry);
            }
            LocalFree((HANDLE)psLBEntry);
#else /* !MSWIN */
            if (longhdr)
                printf(
              "%7lu  %-7s%7lu %4s  %02u-%02u-%02u  %02u:%02u  %08lx  %c",
                  crec.ucsize, method[methnum], csiz, cfactorstr, mo, dy, yr,
                  hh, mm, crec.crc32, (pInfo->lcflag?'^':' '));
            else
                printf("%7lu  %02u-%02u-%02u  %02u:%02u  %c", crec.ucsize,
                  mo, dy, yr, hh, mm, (pInfo->lcflag?'^':' '));
            fnprint((uch *)filename);
#endif /* ?MSWIN */
            error = do_string(crec.file_comment_length, QCOND? DISPLAY : SKIP);
            if (error) {
                error_in_archive = error;  /* might be just warning */
                if (error > PK_WARN)       /* fatal */
                    return error;
            }
            tot_ucsize += crec.ucsize;
            tot_csize += csiz;
            ++members;
#ifdef OS2
            if ((ea_size = SizeOfEAs(extra_field)) != 0) {
                tot_easize += ea_size;
                tot_eafiles++;
            }
#endif
        } else {        /* not listing this file */
            SKIP_(crec.file_comment_length)
        }
    } /* end for-loop (j: files in central directory) */
    if (qflag < 2) {
        if ((cfactor = ratio(tot_ucsize, tot_csize)) < 0) {
            sgn = '-';
            cfactor = (-cfactor + 5) / 10;
        } else {
            sgn = ' ';
            cfactor = (cfactor + 5) / 10;
        }
        sprintf(cfactorstr, "%c%d%%", sgn, cfactor);
#ifdef MSWIN
        if (longhdr)
            wsprintf(lpumb->szTotalsLine,
              "%7lu         %7lu %4s                              %-7u",
              tot_ucsize, tot_csize, cfactorstr, members);
        else
            wsprintf(lpumb->szTotalsLine, "%7lu                    %-7u",
              tot_ucsize, members);
#else /* !MSWIN */
        if (longhdr)
            printf(" ------          ------  ---                \
              -------\n%7lu         %7lu %4s                \
              %-7u\n", tot_ucsize, tot_csize, cfactorstr, members);
        else
            printf(" ------                    -------\n%7lu      \
              %-7u\n", tot_ucsize, members);
#endif /* ?MSWIN */
#ifdef OS2
        if (tot_eafiles && tot_easize)
            printf("\n%ld file%s %ld bytes of EA's attached.\n", tot_eafiles,
              tot_eafiles == 1 ? " has" : "s have a total of", tot_easize);
#endif
    }
    if (readbuf(sig, 4) <= 0)
        return PK_EOF;
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
        error_in_archive = PK_WARN;
    }
    return error_in_archive;
} /* end function list_files() */
static int ratio(uc, c)
    ulg uc, c;
{
    ulg denom;
    if (uc == 0)
        return 0;
    if (uc > 2000000L) {    /* risk signed overflow if multiply numerator */
        denom = uc / 1000L;
        return ((uc >= c) ?
            (int) ((uc-c + (denom>>1)) / denom) :
          -((int) ((c-uc + (denom>>1)) / denom)));
    } else {             /* ^^^^^^^^ rounding */
        denom = uc;
        return ((uc >= c) ?
            (int) ((1000L*(uc-c) + (denom>>1)) / denom) :
          -((int) ((1000L*(c-uc) + (denom>>1)) / denom)));
    }                            /* ^^^^^^^^ rounding */
}
static void fnprint(fn)    /* print filename (after filtering) and newline */
    uch *fn;               /* (GRR:  probably needs EBCDICifying!) */
{
    --fn;
    while (*++fn)
        if (*fn < 32) {     /* ASCII control character */
            putchar('^');
            putchar(*fn+64);
        } else
            putchar(*fn);
    putchar('\n');
} /* end function fnprint() */
