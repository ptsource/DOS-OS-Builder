/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#define FILE_IO_C
#include "unzip.h"
#include "crypt.h"
#include "tables.h"   /* definition/initialization of crc_32_tab[], ebcdic[] */
#ifdef USE_FWRITE  /* see GRR notes below about MS-DOS and 16-bit ints */
#  define WriteError(buf,len,strm) \
          ((ulg)fwrite((char *)(buf),1,(extent)(len),strm) != (ulg)(len))
#else
#  define WriteError(buf,len,strm) \
          ((ulg)write(fileno(strm),(char *)(buf),(extent)(len)) != (ulg)(len))
#endif
static int disk_error OF((void));
int open_input_file()    /* return 1 if open failed */
{
#if defined(UNIX) || defined(TOPS20) || defined(ATARI_ST)
    zipfd = open(zipfn, O_RDONLY);
#else /* !(UNIX || TOPS20) */
#ifdef VMS
    zipfd = open(zipfn, O_RDONLY, 0, "ctx=stm");
#else /* !VMS */
#ifdef MACOS
    zipfd = open(zipfn, 0);
#else /* !MACOS */
    zipfd = open(zipfn, O_RDONLY | O_BINARY);
#endif /* ?MACOS */
#endif /* ?VMS */
#endif /* ?(UNIX || TOPS20) */
    if (zipfd < 0) {
        fprintf(stderr, "error:  can't open zipfile [ %s ]\n", zipfn);
        return 1;
    }
    return 0;
} /* end function open_input_file() */
#ifndef VMS                      /* for VMS use code in vms.c */
int open_outfile()         /* return 1 if fail */
{
#ifdef DOS_NT_OS2
    if (stat(filename, &statbuf) == 0 && !(statbuf.st_mode & S_IWRITE))
        chmod(filename, S_IREAD | S_IWRITE);
#endif
#ifdef UNIX
    if (stat(filename, &statbuf) == 0 && unlink(filename) < 0) {
        fprintf(stderr, "\nerror:  cannot delete old %s\n", filename);
        return 1;
    }
#endif
#ifdef TOPS20
    char *tfilnam;
    if ((tfilnam = (char *)malloc(2*strlen(filename)+1)) == NULL)
        return 1;
    strcpy(tfilnam, filename);
    upper(tfilnam);
    enquote(tfilnam);
    if ((outfile = fopen(tfilnam, FOPW)) == NULL) {
        fprintf(stderr, "\nerror:  cannot create %s\n", tfilnam);
        free(tfilnam);
        return 1;
    }
    free(tfilnam);
#else
#ifdef MTS
    if (aflag)
        outfile = fopen(filename, FOPWT);
    else
        outfile = fopen(filename, FOPW);
    if (outfile == NULL) {
        fprintf(stderr, "\nerror:  cannot create %s\n", filename);
        return 1;
    }
#else
    if ((outfile = fopen(filename, FOPW)) == NULL) {
        fprintf(stderr, "\nerror:  cannot create %s\n", filename);
        return 1;
    }
#endif
#endif
#if 0      /* this SUCKS!  on Ultrix, it must be writing a byte at a time... */
    setbuf(outfile, (char *)NULL);   /* make output unbuffered */
#endif
#ifdef USE_FWRITE
#ifdef DOS_NT_OS2
    setbuf(outfile, (char *)NULL);   /* make output unbuffered */
#else /* !DOS_NT_OS2 */
#ifdef _IOFBF  /* make output fully buffered (works just about like write()) */
    setvbuf(outfile, (char *)slide, _IOFBF, WSIZE);
#else
    setbuf(outfile, (char *)slide);
#endif
#endif /* ?DOS_NT_OS2 */
#endif /* USE_FWRITE */
    return 0;
} /* end function open_outfile() */
#endif /* !VMS */
int readbuf(buf, size)   /* return number of bytes read into buf */
    char *buf;
    register unsigned size;
{
    register int count;
    int n;
    n = size;
    while (size) {
        if (incnt == 0) {
#ifdef OLD_READBUF
            if ((incnt = read(zipfd, (char *)inbuf, INBUFSIZ)) <= 0)
                return (int)(n-size);
#else
            if ((incnt = read(zipfd, (char *)inbuf, INBUFSIZ)) == 0)
                return (int)(n-size);
            else if (incnt < 0) {
                fprintf(stderr, "error:  zipfile read error\n");
                return -1;  /* discarding some data, but better than lockup */
            }
#endif
            cur_zipfile_bufstart += INBUFSIZ;
            inptr = inbuf;
        }
        count = MIN(size, (unsigned)incnt);
        memcpy(buf, inptr, count);
        buf += count;
        inptr += count;
        incnt -= count;
        size -= count;
    }
    return n;
} /* end function readbuf() */
int readbyte()   /* refill inbuf and return a byte if available, else EOF */
{
    if (mem_mode || (incnt = read(zipfd,(char *)inbuf,INBUFSIZ)) <= 0)
        return EOF;
    cur_zipfile_bufstart += INBUFSIZ;   /* always starts on a block boundary */
    inptr = inbuf;
#ifdef CRYPT
    if (pInfo->encrypted) {
        uch *p;
        int n;
        for (n = (long)incnt > csize + 1 ? (int)csize + 1 : incnt,
             p = inptr;  n--;  p++)
            zdecode(*p);
    }
#endif /* CRYPT */
    --incnt;
    return *inptr++;
} /* end function readbyte() */
#ifndef VMS                 /* for VMS use code in vms.c */
int flush(rawbuf, size, unshrink)   /* cflag => always 0; 50 if write error */
    uch *rawbuf;
    ulg size;
    int unshrink;
{
    register ulg crcval = crc32val;
    register ulg n = size;
    register uch *p=rawbuf, *q;
    uch *transbuf;
    ulg transbufsiz;
    static int didCRlast = FALSE;
    while (n--)
        crcval = crc_32_tab[((uch)crcval ^ (*p++)) & 0xff] ^ (crcval >> 8);
    crc32val = crcval;
    if (tflag || size == 0L)   /* testing or nothing to write:  all done */
        return 0;
    if (disk_full)
        return 50;            /* disk already full:  ignore rest of file */
    if (!pInfo->textmode) {
        if (WriteError(rawbuf, size, outfile))  /* write raw binary data */
            return cflag? 0 : disk_error();
    } else {
        if (unshrink) {
            transbuf = outbuf2;
            transbufsiz = TRANSBUFSIZ;
        } else {
            transbuf = outbuf;
            transbufsiz = OUTBUFSIZ;
            Trace((stderr, "\ntransbufsiz = OUTBUFSIZ = %u\n", OUTBUFSIZ));
        }
        if (newfile) {
            didCRlast = FALSE;   /* no previous buffers written */
            newfile = FALSE;
        }
        p = rawbuf;
        if (*p == LF && didCRlast)
            ++p;
        for (didCRlast = FALSE, q = transbuf;  p < rawbuf+size;  ++p) {
            if (*p == CR) {              /* lone CR or CR/LF: EOL either way */
                PutNativeEOL
                if (p == rawbuf+size-1)  /* last char in buffer */
                    didCRlast = TRUE;
                else if (p[1] == LF)     /* get rid of accompanying LF */
                    ++p;
            } else if (*p == LF)         /* lone LF */
                PutNativeEOL
            else
#ifndef DOS_NT_OS2
            if (*p != CTRLZ)             /* lose all ^Z's */
#endif
                *q++ = native(*p);
#if (defined(SMALL_MEM) || defined(MED_MEM))
# if (lenEOL == 1)   /* don't check unshrink:  both buffers small but equal */
            if (!unshrink)
# endif
                if (q > transbuf+transbufsiz-lenEOL) {
                    Trace((stderr,
                      "p - rawbuf = %u   q-transbuf = %u   size = %lu\n",
                      (unsigned)(p-rawbuf), (unsigned)(q-transbuf), size));
                    if (WriteError(transbuf, (unsigned)(q-transbuf), outfile))
                        return cflag? 0 : disk_error();
                    q = transbuf;
                    continue;
                }
#endif /* SMALL_MEM || MED_MEM */
        }
        Trace((stderr, "p - rawbuf = %u   q-transbuf = %u   size = %lu\n",
          (unsigned)(p-rawbuf), (unsigned)(q-transbuf), size));
        if (q > transbuf &&
            WriteError(transbuf, (unsigned)(q-transbuf), outfile))
            return cflag? 0 : disk_error();
    }
    return 0;
} /* end function flush() */
static int disk_error()
{
    fprintf(stderr, "\n%s:  write error (disk full?).  Continue? (y/n/^C) ",
      filename);
    FFLUSH(stderr);
#ifndef MSWIN
    fgets(answerbuf, 9, stdin);
    if (*answerbuf == 'y')   /* stop writing to this file */
        disk_full = 1;       /*  (outfile bad?), but new OK */
    else
#endif
        disk_full = 2;       /* no:  exit program */
    return 50;               /* 50:  disk full */
} /* end function disk_error() */
#endif /* !VMS */
void handler(signal)   /* upon interrupt, turn on echo and exit cleanly */
    int signal;
{
#if defined(SIGBUS) || defined(SIGSEGV)
    static char *corrupt = "error:  zipfile probably corrupt (%s)\n";
#endif
#if !defined(DOS_NT_OS2) && !defined(MACOS)
    echon();
    putc('\n', stderr);
#endif /* !DOS_NT_OS2 && !MACOS */
#ifdef SIGBUS
    if (signal == SIGBUS) {
        fprintf(stderr, corrupt, "bus error");
        exit(3);
    }
#endif /* SIGBUS */
#ifdef SIGSEGV
    if (signal == SIGSEGV) {
        fprintf(stderr, corrupt, "segmentation violation");
        exit(3);
    }
#endif /* SIGSEGV */
    exit(0);
}
#ifndef VMS
time_t dos_to_unix_time(ddate, dtime)
    unsigned ddate, dtime;
{
    int yr, mo, dy, hh, mm, ss;
#ifdef TOPS20
#   define YRBASE  1900
    struct tmx *tmx;
    char temp[20];
    time_t retval;
#else /* !TOPS20 */
#   define YRBASE  1970
    static short yday[]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    int leap;
    long m_time, days=0;
#if (!defined(MACOS) && !defined(MSC))
#if (defined(BSD) || defined(MTS) || defined(__GO32__))
#ifndef __386BSD__
    static struct timeb tbp;
#endif /* __386BSD__ */
#else /* !(BSD || MTS || __GO32__) */
#ifdef ATARI_ST
    extern long _timezone;
#   define timezone _timezone;  /* a whoops in our library... */
#else /* !ATARI_ST */
    extern long timezone;       /* declared in <time.h> for MSC (& Borland?) */
#endif /* ?ATARI_ST */
#endif /* ?(BSD || MTS || __GO32__) */
#endif /* !MACOS && !MSC */
#endif /* ?TOPS20 */
    yr = ((ddate >> 9) & 0x7f) + (1980 - YRBASE);
    mo = ((ddate >> 5) & 0x0f) - 1;
    dy = (ddate & 0x1f) - 1;
    hh = (dtime >> 11) & 0x1f;
    mm = (dtime >> 5) & 0x3f;
    ss = (dtime & 0x1f) * 2;
#ifdef TOPS20
    tmx = (struct tmx *)malloc(sizeof(struct tmx));
    sprintf (temp, "%02d/%02d/%02d %02d:%02d:%02d", mo+1, dy+1, yr, hh, mm, ss);
    time_parse(temp, tmx, (char *)0);
    retval = time_make(tmx);
    free(tmx);
    return retval;
#else /* !TOPS20 */
    leap = ((yr + YRBASE - 1) / 4);   /* leap year base factor */
    days = (yr * 365) + (leap - 492) + yday[mo];
    if ((mo > 1) && ((yr+YRBASE)%4 == 0) && ((yr+YRBASE) != 2100))
        ++days;                 /* OK through 2199 */
    m_time = ((long)(days + dy) * 86400L) + ((long)hh * 3600) + (mm * 60) + ss;
#ifndef MACOS
#if (defined(BSD) || defined(MTS) || defined(__GO32__))
#if (!defined(__386BSD__) && !defined(__bsdi__))
    ftime(&tbp);
    m_time += tbp.timezone * 60L;
#endif
#else /* !(BSD || MTS || __GO32__) */
#ifdef WIN32
    {
        TIME_ZONE_INFORMATION tzinfo;
        DWORD res;
        res = GetTimeZoneInformation(&tzinfo);
        if (res == TIME_ZONE_ID_STANDARD)
            m_time += 60*tzinfo.StandardBias;
        else if (res == TIME_ZONE_ID_DAYLIGHT)
            m_time += 60*tzinfo.DaylightBias;
    }
#else /* !WIN32 */
    tzset();                    /* set `timezone' variable */
    m_time += timezone;
#endif /* ?WIN32 */
#endif /* ?(BSD || MTS || __GO32__) */
#endif /* !MACOS */
#ifdef __386BSD__               /* see comments in unix.c */
    m_time -= localtime((time_t *) &m_time)->tm_gmtoff;
#else /* !__386BSD__ */
#ifndef WIN32
    if (localtime((time_t *)&m_time)->tm_isdst)
        m_time -= 60L * 60L;    /* adjust for daylight savings time */
#endif /* !WIN32 */
#endif /* ?__386BSD__ */
    return m_time;
#endif /* ?TOPS20 */
} /* end function dos_to_unix_time() */
#endif /* !VMS */
#if !defined(VMS) && !defined(OS2)
int check_for_newer(filename)   /* return 1 if existing file newer or equal; */
    char *filename;             /*  0 if older; -1 if doesn't exist yet */
{
    time_t existing, archive;
    if (stat(filename, &statbuf))
        return DOES_NOT_EXIST;
    existing = (statbuf.st_mtime & 1) ? statbuf.st_mtime+1 : statbuf.st_mtime;
    archive  = dos_to_unix_time(lrec.last_mod_file_date,
                                lrec.last_mod_file_time);
#ifdef PRINT_TIME
    fprintf(stderr, "existing %ld, archive %ld, e-a %ld\n", existing, archive,
      existing-archive);
#endif
    return (existing >= archive);
} /* end function check_for_newer() */
#endif /* !VMS && !OS2 */
int find_end_central_dir(searchlen)   /* return PK-class error */
    long searchlen;
{
    int i, numblks, found=FALSE;
    LONGINT tail_len;
    ec_byte_rec byterec;
    if (ziplen <= INBUFSIZ) {
        lseek(zipfd, 0L, SEEK_SET);
        if ((incnt = read(zipfd,(char *)inbuf,(unsigned int)ziplen)) ==
             (int)ziplen)
            for (inptr = inbuf+(int)ziplen-22;  inptr >= inbuf;  --inptr)
                if ((native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) {
                    incnt -= inptr - inbuf;
                    found = TRUE;
                    break;
                }
    } else {
        if ((tail_len = ziplen % INBUFSIZ) > ECREC_SIZE) {
            cur_zipfile_bufstart = lseek(zipfd, ziplen-tail_len, SEEK_SET);
            if ((incnt = read(zipfd,(char *)inbuf,(unsigned int)tail_len)) !=
                 (int)tail_len)
                goto fail;      /* shut up; it's expedient */
            for (inptr = inbuf+(int)tail_len-22;  inptr >= inbuf;  --inptr)
                if ((native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) {
                    incnt -= inptr - inbuf;
                    found = TRUE;
                    break;
                }
            strncpy((char *)hold, (char *)inbuf, 3);
        } else
            cur_zipfile_bufstart = ziplen - tail_len;
        numblks = (int)((searchlen - tail_len + (INBUFSIZ-1)) / INBUFSIZ);
        for (i = 1;  !found && (i <= numblks);  ++i) {
            cur_zipfile_bufstart -= INBUFSIZ;
            lseek(zipfd, cur_zipfile_bufstart, SEEK_SET);
            if ((incnt = read(zipfd,(char *)inbuf,INBUFSIZ)) != INBUFSIZ)
                break;          /* fall through and fail */
            for (inptr = inbuf+INBUFSIZ-1;  inptr >= inbuf;  --inptr)
                if ((native(*inptr) == 'P')  &&
                     !strncmp((char *)inptr, end_central_sig, 4)) {
                    incnt -= inptr - inbuf;
                    found = TRUE;
                    break;
                }
            strncpy((char *)hold, (char *)inbuf, 3);
        }
    } /* end if (ziplen > INBUFSIZ) */
fail:
    if (!found) {
#ifdef MSWIN
        MessageBeep(1);
#endif
        if (qflag || (zipinfo_mode && !hflag))
            fprintf(stderr, "[%s]\n", zipfn);
        fprintf(stderr, "\
  End-of-central-directory signature not found.  Either this file is not\n\
  a zipfile, or it constitutes one disk of a multi-part archive.  In the\n\
  latter case the central directory and zipfile comment will be found on\n\
  the last disk(s) of this archive.\n\n");
        return PK_ERR;   /* failed */
    }
    real_ecrec_offset = cur_zipfile_bufstart + (inptr-inbuf);
#ifdef TEST
    printf("\n  found end-of-central-dir signature at offset %ld (%.8lXh)\n",
      real_ecrec_offset, real_ecrec_offset);
    printf("    from beginning of file; offset %d (%.4Xh) within block\n",
      inptr-inbuf, inptr-inbuf);
#endif
    if (readbuf((char *)byterec, ECREC_SIZE+4) <= 0)
        return PK_EOF;
    ecrec.number_this_disk =
        makeword(&byterec[NUMBER_THIS_DISK]);
    ecrec.num_disk_with_start_central_dir =
        makeword(&byterec[NUM_DISK_WITH_START_CENTRAL_DIR]);
    ecrec.num_entries_centrl_dir_ths_disk =
        makeword(&byterec[NUM_ENTRIES_CENTRL_DIR_THS_DISK]);
    ecrec.total_entries_central_dir =
        makeword(&byterec[TOTAL_ENTRIES_CENTRAL_DIR]);
    ecrec.size_central_directory =
        makelong(&byterec[SIZE_CENTRAL_DIRECTORY]);
    ecrec.offset_start_central_directory =
        makelong(&byterec[OFFSET_START_CENTRAL_DIRECTORY]);
    ecrec.zipfile_comment_length =
        makeword(&byterec[ZIPFILE_COMMENT_LENGTH]);
    expect_ecrec_offset = ecrec.offset_start_central_directory +
                          ecrec.size_central_directory;
    return PK_COOL;
} /* end function find_end_central_dir() */
int get_cdir_file_hdr()   /* return PK-type error code */
{
    cdir_byte_hdr byterec;
    if (readbuf((char *)byterec, CREC_SIZE) <= 0)
        return PK_EOF;
    crec.version_made_by[0] = byterec[C_VERSION_MADE_BY_0];
    crec.version_made_by[1] = byterec[C_VERSION_MADE_BY_1];
    crec.version_needed_to_extract[0] = byterec[C_VERSION_NEEDED_TO_EXTRACT_0];
    crec.version_needed_to_extract[1] = byterec[C_VERSION_NEEDED_TO_EXTRACT_1];
    crec.general_purpose_bit_flag =
        makeword(&byterec[C_GENERAL_PURPOSE_BIT_FLAG]);
    crec.compression_method =
        makeword(&byterec[C_COMPRESSION_METHOD]);
    crec.last_mod_file_time =
        makeword(&byterec[C_LAST_MOD_FILE_TIME]);
    crec.last_mod_file_date =
        makeword(&byterec[C_LAST_MOD_FILE_DATE]);
    crec.crc32 =
        makelong(&byterec[C_CRC32]);
    crec.csize =
        makelong(&byterec[C_COMPRESSED_SIZE]);
    crec.ucsize =
        makelong(&byterec[C_UNCOMPRESSED_SIZE]);
    crec.filename_length =
        makeword(&byterec[C_FILENAME_LENGTH]);
    crec.extra_field_length =
        makeword(&byterec[C_EXTRA_FIELD_LENGTH]);
    crec.file_comment_length =
        makeword(&byterec[C_FILE_COMMENT_LENGTH]);
    crec.disk_number_start =
        makeword(&byterec[C_DISK_NUMBER_START]);
    crec.internal_file_attributes =
        makeword(&byterec[C_INTERNAL_FILE_ATTRIBUTES]);
    crec.external_file_attributes =
        makelong(&byterec[C_EXTERNAL_FILE_ATTRIBUTES]);  /* LONG, not word! */
    crec.relative_offset_local_header =
        makelong(&byterec[C_RELATIVE_OFFSET_LOCAL_HEADER]);
    return PK_COOL;
} /* end function get_cdir_file_hdr() */
int do_string(len, option)      /* return PK-type error code */
    unsigned int len;           /* without prototype, ush converted to this */
    int option;
{
    long comment_bytes_left, block_length;
    int error=PK_OK;
    ush extra_len;
    if (!len)
        return PK_COOL;
    switch (option) {
    case DISPLAY:
        comment_bytes_left = len;
        block_length = OUTBUFSIZ;    /* for the while statement, first time */
        while (comment_bytes_left > 0 && block_length > 0) {
#ifndef MSWIN
            register uch *p = outbuf-1;
#endif
            if ((block_length = readbuf((char *)outbuf,
                   (int) MIN(OUTBUFSIZ, comment_bytes_left))) <= 0)
                return PK_EOF;
            comment_bytes_left -= block_length;
            NUKE_CRs(outbuf, block_length);   /* (modifies block_length) */
            outbuf[block_length] = '\0';   /* terminate w/zero:  ASCIIZ */
            A_TO_N(outbuf);   /* translate string to native */
#ifdef MSWIN
            WriteStringToMsgWin(outbuf, bRealTimeMsgUpdate);
#else /* !MSWIN */
#ifdef NATIVE
            printf("%s", outbuf);   /* GRR:  can ANSI be used with EBCDIC? */
#else /* ASCII */
            while (*++p)
                if (*p == 0x1B) {   /* ASCII escape char */
                    putchar('^');
                    putchar('[');
                } else
                    putchar(*p);
#endif /* ?NATIVE */
#endif /* ?MSWIN */
        }
        printf("\n");   /* assume no newline at end */
        break;
    case FILENAME:
        extra_len = 0;
        if (len >= FILNAMSIZ) {
            fprintf(stderr, "warning:  filename too long--truncating.\n");
            error = PK_WARN;
            extra_len = len - FILNAMSIZ + 1;
            len = FILNAMSIZ - 1;
        }
        if (readbuf(filename, len) <= 0)
            return PK_EOF;
        filename[len] = '\0';   /* terminate w/zero:  ASCIIZ */
        A_TO_N(filename);       /* translate string to native */
        if (pInfo->lcflag)      /* replace with lowercase filename */
            TOLOWER(filename, filename);
        if (pInfo->vollabel && len > 8 && filename[8] == '.') {
            char *p = filename+8;
            while (*p++)
                p[-1] = *p;  /* disk label, and 8th char is dot:  remove dot */
        }
        if (!extra_len)         /* we're done here */
            break;
        fprintf(stderr, "[ %s ]\n", filename);
        len = extra_len;
    case SKIP:
        LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        break;
    case EXTRA_FIELD:
        if (extra_field != (uch *)NULL)
            free(extra_field);
        if ((extra_field = (uch *)malloc(len)) == (uch *)NULL) {
            fprintf(stderr,
              "warning:  extra field too long (%d).  Ignoring...\n", len);
            LSEEK(cur_zipfile_bufstart + (inptr-inbuf) + len)
        } else
            if (readbuf((char *)extra_field, len) <= 0)
                return PK_EOF;
        break;
    } /* end switch (option) */
    return error;
} /* end function do_string() */
ush makeword(b)
    uch *b;
{
    return (ush)((b[1] << 8) | b[0]);
}
ulg makelong(sig)
    uch *sig;
{
    return (((ulg)sig[3]) << 24)
        + (((ulg)sig[2]) << 16)
        + (((ulg)sig[1]) << 8)
        + ((ulg)sig[0]);
}
#ifdef ZMEM   /* memset, memcpy for systems without them */
char *memset(buf, init, len)
    register char *buf, init;   /* buffer loc and initializer */
    register unsigned int len;  /* length of the buffer */
{
    char *start;
    start = buf;
    while (len--)
        *(buf++) = init;
    return start;
}
char *memcpy(dst, src, len)
    register char *dst, *src;
    register unsigned int len;
{
    char *start;
    start = dst;
    while (len-- > 0)
        *dst++ = *src++;
    return start;
}
#endif /* ZMEM */
int zstrnicmp(s1, s2, n)
    register char *s1, *s2;
    register int n;
{
    for (; n > 0;  --n, ++s1, ++s2) {
        if (ToLower(*s1) != ToLower(*s2))
            return (ToLower(*s1) < ToLower(*s2))? -1 : 1;
        if (*s1 == '\0')   /* both strings terminate early */
            return 0;
    }
    return 0;
}
