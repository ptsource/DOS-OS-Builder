/********************************************************/
/*                                                      */
/*               www.wiki.ptsource.eu                   */
/*                                                      */
/********************************************************/
#include "unzip.h"
#include "crypt.h"
#ifdef MSWIN
#  include "wizunzip.h"
#  include "replace.h"
#endif
int newfile;   /* used also in file_io.c (flush()) */
static int store_info __((void));
static int extract_or_test_member __((void));
static char *VersionMsg =
  "   skipping: %-22s  need %s compat. v%u.%u (can do v%u.%u)\n";
static char *ComprMsg =
  "   skipping: %-22s  compression method %d\n";
static char *FilNamMsg =
  "%s:  bad filename length (%s)\n";
static char *ExtFieldMsg =
  "%s:  bad extra field length (%s)\n";
static char *OffsetMsg =
  "file #%d:  bad zipfile offset (%s):  %ld\n";
static char *ExtractMsg =
  "%11s: %-22s  %s%s";
static char *LengthMsg =
  "%s  %s:  %ld bytes required to uncompress to %lu bytes;\n       %s\
   supposed to require %lu bytes%s%s%s\n";
int extract_or_test_files()    /* return PK-type error code */
{
    uch *cd_inptr;
    int cd_incnt, error, error_in_archive=PK_COOL;
    int i, j, renamed, query, len, filnum=(-1), blknum=0;
    int *fn_matched=NULL, *xn_matched=NULL;
    ush members_remaining, num_skipped=0, num_bad_pwd=0;
    long cd_bufstart, bufstart, inbuf_offset, request;
    static min_info info[DIR_BLKSIZ];
    pInfo = info;
    members_remaining = ecrec.total_entries_central_dir;
#ifdef CRYPT
    newzip = TRUE;
#endif
    if (filespecs > 0  &&
        (fn_matched=(int *)malloc(filespecs*sizeof(int))) != NULL)
        for (i = 0;  i < filespecs;  ++i)
            fn_matched[i] = FALSE;
    if (xfilespecs > 0  &&
        (xn_matched=(int *)malloc(xfilespecs*sizeof(int))) != NULL)
        for (i = 0;  i < xfilespecs;  ++i)
            xn_matched[i] = FALSE;
    while (members_remaining) {
        j = 0;
        while (members_remaining && (j < DIR_BLKSIZ)) {
            --members_remaining;
            pInfo = &info[j];
            if (readbuf(sig, 4) <= 0) {
                error_in_archive = PK_EOF;
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            if (strncmp(sig, central_hdr_sig, 4)) {  /* just to make sure */
                fprintf(stderr, CentSigMsg, j);  /* sig not found */
                fprintf(stderr, ReportMsg);   /* check binary transfers */
                error_in_archive = PK_BADERR;
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            if ((error = process_cdir_file_hdr()) != PK_COOL) {
                error_in_archive = error;   /* only PK_EOF defined */
                members_remaining = 0;  /* ...so no more left to do */
                break;
            }
            if ((error = do_string(crec.filename_length,FILENAME)) != PK_COOL) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > PK_WARN) {  /* fatal:  no more left to do */
                    fprintf(stderr, FilNamMsg, filename, "central");
                    members_remaining = 0;
                    break;
                }
            }
            if ((error = do_string(crec.extra_field_length, EXTRA_FIELD)) != 0)
            {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > PK_WARN) {  /* fatal */
                    fprintf(stderr, ExtFieldMsg, filename, "central");
                    members_remaining = 0;
                    break;
                }
            }
            if ((error = do_string(crec.file_comment_length,SKIP)) != PK_COOL) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > PK_WARN) {  /* fatal */
                    fprintf(stderr, "\n%s:  bad file comment length\n",
                            filename);
                    members_remaining = 0;
                    break;
                }
            }
            if (process_all_files) {
                if (store_info())
                    ++j;  /* file is OK; info[] stored; continue with next */
                else
                    ++num_skipped;
            } else {
                int   do_this_file = FALSE;
                char  **pfn = pfnames-1;
                while (*++pfn)
                    if (match(filename, *pfn, pInfo->lcflag)) {
                        do_this_file = TRUE;
                        if (fn_matched)
                            fn_matched[pfn-pfnames] = TRUE;
                        break;       /* found match, so stop looping */
                    }
                if (do_this_file) {  /* check if this is an excluded file */
                    char  **pxn = pxnames-1;
                    while (*++pxn)
                        if (match(filename, *pxn, pInfo->lcflag)) {
                            do_this_file = FALSE;
                            if (xn_matched)
                                xn_matched[pxn-pxnames] = TRUE;
                            break;
                        }
                }
                if (do_this_file)
                    if (store_info())
                        ++j;            /* file is OK */
                    else
                        ++num_skipped;  /* unsupp. compression or encryption */
            } /* end if (process_all_files) */
        } /* end while-loop (adding files to current block) */
        cd_bufstart = cur_zipfile_bufstart;
        cd_inptr = inptr;
        cd_incnt = incnt;
        for (i = 0; i < j; ++i) {
            filnum = i + blknum*DIR_BLKSIZ;
            pInfo = &info[i];
            request = pInfo->offset + extra_bytes;
            inbuf_offset = request % INBUFSIZ;
            bufstart = request - inbuf_offset;
            if (request < 0) {
                fprintf(stderr, SeekMsg, zipfn, ReportMsg);
                error_in_archive = PK_BADERR;
                continue;   /* but can still go on */
            } else if (bufstart != cur_zipfile_bufstart) {
                cur_zipfile_bufstart = lseek(zipfd,(LONGINT)bufstart,SEEK_SET);
                if ((incnt = read(zipfd,(char *)inbuf,INBUFSIZ)) <= 0) {
                    fprintf(stderr, OffsetMsg, filnum, "lseek", bufstart);
                    error_in_archive = PK_BADERR;
                    continue;   /* can still do next file */
                }
                inptr = inbuf + (int)inbuf_offset;
                incnt -= (int)inbuf_offset;
            } else {
                incnt += (inptr-inbuf) - (int)inbuf_offset;
                inptr = inbuf + (int)inbuf_offset;
            }
            if (readbuf(sig, 4) <= 0) {  /* bad offset */
                fprintf(stderr, OffsetMsg, filnum, "EOF", request);
                error_in_archive = PK_BADERR;
                continue;   /* but can still try next one */
            }
            if (strncmp(sig, local_hdr_sig, 4)) {
                fprintf(stderr, OffsetMsg, filnum, "local header sig", request);
                error_in_archive = PK_ERR;
                if (filnum == 0) {
                    fprintf(stderr, "  (attempting to re-compensate)\n");
                    extra_bytes = 0L;
                    LSEEK(pInfo->offset)
                    if (readbuf(sig, 4) <= 0) {  /* bad offset */
                        fprintf(stderr, OffsetMsg, filnum, "EOF", request);
                        error_in_archive = PK_BADERR;
                        continue;   /* but can still try next one */
                    }
                    if (strncmp(sig, local_hdr_sig, 4)) {
                        fprintf(stderr, OffsetMsg, filnum, "local header sig",
                          request);
                        error_in_archive = PK_BADERR;
                        continue;
                    }
                }
            }
            if ((error = process_local_file_hdr()) != PK_COOL) {
                fprintf(stderr, "\nfile #%d:  bad local header\n", filnum);
                error_in_archive = error;   /* only PK_EOF defined */
                continue;   /* can still try next one */
            }
            if ((error = do_string(lrec.filename_length,FILENAME)) != PK_COOL) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > PK_WARN) {
                    fprintf(stderr, FilNamMsg, filename, "local");
                    continue;   /* go on to next one */
                }
            }
            if (extra_field != (uch *)NULL) {
                free(extra_field);
                extra_field = (uch *)NULL;
            }
            if ((error = do_string(lrec.extra_field_length,EXTRA_FIELD)) != 0) {
                if (error > error_in_archive)
                    error_in_archive = error;
                if (error > PK_WARN) {
                    fprintf(stderr, ExtFieldMsg, filename, "local");
                    continue;   /* go on */
                }
            }
            if (!tflag && !cflag) {
                renamed = FALSE;   /* user hasn't renamed output file yet */
startover:
                query = FALSE;
#ifdef MACOS
                macflag = (pInfo->hostnum == MAC_);
#endif
                if ((error = mapname(renamed)) > PK_WARN) {
                    if (error == IZ_CREATED_DIR) {
                    } else if (error == IZ_VOL_LABEL) {
                        fprintf(stderr,
                          "   skipping: %-22s  %svolume label\n", filename,
#ifdef DOS_NT_OS2
                          volflag? "hard disk " :
#endif
                          "");
                    } else if (error > PK_ERR  &&  error_in_archive < PK_ERR)
                        error_in_archive = PK_ERR;
                    Trace((stderr, "mapname(%s) returns error = %d\n", filename,
                      error));
                    continue;   /* go on to next file */
                }
                switch (check_for_newer(filename)) {
                    case DOES_NOT_EXIST:
                        if (fflag && !renamed)  /* don't skip if just renamed */
                            continue;   /* freshen (no new files):  skip */
                        break;
                    case EXISTS_AND_OLDER:
                        if (overwrite_none)
                            continue;   /* never overwrite:  skip file */
                        if (!overwrite_all && !force_flag)
                            query = TRUE;
                        break;
                    case EXISTS_AND_NEWER:             /* (or equal) */
                        if (overwrite_none || (uflag && !renamed))
                            continue;  /* skip if update/freshen & orig name */
                        if (!overwrite_all && !force_flag)
                            query = TRUE;
                        break;
                }
                if (query) {
#ifdef MSWIN
                    FARPROC lpfnprocReplace;
                    int ReplaceDlgRetVal;   /* replace dialog return value */
                    ShowCursor(FALSE);      /* turn off cursor */
                    SetCursor(hSaveCursor); /* restore the cursor */
                    lpfnprocReplace = MakeProcInstance(ReplaceProc, hInst);
                    ReplaceDlgRetVal = DialogBoxParam(hInst, "Replace",
                      hWndMain, lpfnprocReplace, (DWORD)(LPSTR)filename);
                    FreeProcInstance(lpfnprocReplace);
                    hSaveCursor = SetCursor(hHourGlass);
                    ShowCursor(TRUE);
                    switch (ReplaceDlgRetVal) {
                        case IDM_REPLACE_RENAME:
                            renamed = TRUE;
                            goto startover;
                        case IDM_REPLACE_YES:
                            break;
                        case IDM_REPLACE_ALL:
                            overwrite_all = TRUE;
                            overwrite_none = FALSE;  /* just to make sure */
                            break;
                        case IDM_REPLACE_NONE:
                            overwrite_none = TRUE;
                            overwrite_all = FALSE;  /* make sure */
                            force_flag = FALSE;     /* ditto */
                        case IDM_REPLACE_NO:
                            continue;
                    }
#else /* !MSWIN */
reprompt:
                    fprintf(stderr,
                      "replace %s? [y]es, [n]o, [A]ll, [N]one, [r]ename: ",
                      filename);
                    FFLUSH(stderr);
                    if (fgets(answerbuf, 9, stdin) == NULL) {
                        fprintf(stderr, " NULL\n(assuming [N]one)\n");
                        FFLUSH(stderr);
                        *answerbuf = 'N';
                        if (!error_in_archive)
                            error_in_archive = 1;  /* not extracted:  warning */
                    }
                    switch (*answerbuf) {
                        case 'A':   /* dangerous option:  force caps */
                            overwrite_all = TRUE;
                            overwrite_none = FALSE;  /* just to make sure */
                            break;
                        case 'r':
                        case 'R':
                            do {
                                fprintf(stderr, "new name: ");
                                FFLUSH(stderr);
                                fgets(filename, FILNAMSIZ, stdin);
                                len = strlen(filename);
                                if (filename[len-1] == '\n')
                                    filename[--len] = 0;
                            } while (len == 0);
                            renamed = TRUE;
                            goto startover;   /* sorry for a goto */
                        case 'y':
                        case 'Y':
                            break;
                        case 'N':
                            overwrite_none = TRUE;
                            overwrite_all = FALSE;  /* make sure */
                            force_flag = FALSE;     /* ditto */
                        case 'n':
                            continue;   /* skip file */
                        default:
                            fprintf(stderr, "error:  invalid response [%c]\n",
                              *answerbuf);   /* warn the user */
                            goto reprompt;   /* why not another goto? */
                    } /* end switch (*answerbuf) */
#endif /* ?MSWIN */
                } /* end if (query) */
            } /* end if (extracting to disk) */
#ifdef CRYPT
            if (pInfo->encrypted && (error = decrypt()) != PK_COOL) {
                if (error == PK_MEM2) {
                    if (error > error_in_archive)
                        error_in_archive = error;
                    fprintf(stderr,
                      "   skipping: %-22s  unable to get password\n", filename);
                } else {  /* (error == PK_WARN) */
                    fprintf(stderr,
                      "   skipping: %-22s  incorrect password\n", filename);
                    ++num_bad_pwd;
                }
                continue;   /* go on to next file */
            }
#endif /* CRYPT */
            disk_full = 0;
            if ((error = extract_or_test_member()) != PK_COOL) {
                if (error > error_in_archive)
                    error_in_archive = error;       /* ...and keep going */
                if (disk_full > 1)
                    return error_in_archive;        /* (unless disk full) */
            }
        } /* end for-loop (i:  files in current block) */
        cur_zipfile_bufstart = lseek(zipfd, (LONGINT)cd_bufstart, SEEK_SET);
        read(zipfd, (char *)inbuf, INBUFSIZ);  /* were there b4 ==> no error */
        inptr = cd_inptr;
        incnt = cd_incnt;
        ++blknum;
#ifdef TEST
        printf("\ncd_bufstart = %ld (%.8lXh)\n", cd_bufstart, cd_bufstart);
        printf("cur_zipfile_bufstart = %ld (%.8lXh)\n", cur_zipfile_bufstart,
          cur_zipfile_bufstart);
        printf("inptr-inbuf = %d\n", inptr-inbuf);
        printf("incnt = %d\n\n", incnt);
#endif
    } /* end while-loop (blocks of files in central directory) */
    if (fn_matched) {
        for (i = 0;  i < filespecs;  ++i)
            if (!fn_matched[i])
                fprintf(stderr, "caution: filename not matched:  %s\n",
                  pfnames[i]);
        free(fn_matched);
    }
    if (xn_matched) {
        for (i = 0;  i < xfilespecs;  ++i)
            if (!xn_matched[i])
                fprintf(stderr, "caution: excluded filename not matched:  %s\n",
                  pxnames[i]);
        free(xn_matched);
    }
    if (readbuf(sig, 4) <= 0)
        error_in_archive = PK_EOF;
    if (strncmp(sig, end_central_sig, 4)) {     /* just to make sure again */
        fprintf(stderr, EndSigMsg);  /* didn't find end-of-central-dir sig */
        fprintf(stderr, ReportMsg);  /* check binary transfers */
        if (!error_in_archive)       /* don't overwrite stronger error */
            error_in_archive = PK_WARN;
    }
    if (tflag) {
        int num=filnum+1 - num_bad_pwd;
        if (qflag < 2) {         /* GRR 930710:  was (qflag == 1) */
            if (error_in_archive)
                printf("At least one %serror was detected in %s.\n",
                  (error_in_archive == 1)? "warning-" : "", zipfn);
            else if (num == 0)
                printf("Caution:  zero files tested in %s.\n", zipfn);
            else if (process_all_files && (num_skipped+num_bad_pwd == 0))
                printf("No errors detected in compressed data of %s.\n", zipfn);
            else
                printf("No errors detected in %s for the %d file%s tested.\n",
                  zipfn, num, (num==1)? "":"s");
            if (num_skipped > 0)
                printf("%d file%s skipped because of unsupported compression \
or encoding.\n", num_skipped, (num_skipped==1)? "":"s");
#ifdef CRYPT
            if (num_bad_pwd > 0)
                printf("%d file%s skipped because of incorrect password.\n",
                  num_bad_pwd, (num_bad_pwd==1)? "":"s");
#endif /* CRYPT */
        } else if ((qflag == 0) && !error_in_archive && (num == 0))
            printf("Caution:  zero files tested in %s.\n", zipfn);
    }
    if ((num_skipped > 0) && !error_in_archive)
        error_in_archive = PK_WARN;
#ifdef CRYPT
    if ((num_bad_pwd > 0) && !error_in_archive)
        error_in_archive = PK_WARN;
#endif /* CRYPT */
    return error_in_archive;
} /* end function extract_or_test_files() */
static int store_info()   /* return 0 if skipping, 1 if OK */
{
#ifdef SFX
#  define UNKN_COMPR \
   (crec.compression_method!=STORED && crec.compression_method!=DEFLATED)
#else
#  define UNKN_COMPR \
   (crec.compression_method>IMPLODED && crec.compression_method!=DEFLATED)
#endif
    pInfo->encrypted = crec.general_purpose_bit_flag & 1;       /* bit field */
    pInfo->ExtLocHdr = (crec.general_purpose_bit_flag & 8) == 8;/* bit field */
    pInfo->textfile = crec.internal_file_attributes & 1;        /* bit field */
    pInfo->crc = crec.crc32;
    pInfo->compr_size = crec.csize;
    switch (aflag) {
        case 0:
            pInfo->textmode = FALSE;   /* bit field */
            break;
        case 1:
            pInfo->textmode = pInfo->textfile;   /* auto-convert mode */
            break;
        default:  /* case 2: */
            pInfo->textmode = TRUE;
            break;
    }
    if (crec.version_needed_to_extract[1] == VMS_) {
        if (crec.version_needed_to_extract[0] > VMS_VERSION) {
            fprintf(stderr, VersionMsg, filename, "VMS",
              crec.version_needed_to_extract[0] / 10,
              crec.version_needed_to_extract[0] % 10,
              VMS_VERSION / 10, VMS_VERSION % 10);
            return 0;
        }
#ifndef VMS   /* won't be able to use extra field, but still have data */
        else if (!tflag && !force_flag) {  /* if forcing, extract regardless */
            fprintf(stderr,
              "\n%s:  stored in VMS format.  Extract anyway? (y/n) ",
              filename);
            FFLUSH(stderr);
            fgets(answerbuf, 9, stdin);
            if ((*answerbuf != 'y') && (*answerbuf != 'Y'))
                return 0;
        }
#endif /* !VMS */
    } else if (crec.version_needed_to_extract[0] > UNZIP_VERSION) {
        fprintf(stderr, VersionMsg, filename, "PK",
          crec.version_needed_to_extract[0] / 10,
          crec.version_needed_to_extract[0] % 10,
          UNZIP_VERSION / 10, UNZIP_VERSION % 10);
        return 0;
    }
    if UNKN_COMPR {
        fprintf(stderr, ComprMsg, filename, crec.compression_method);
        return 0;
    }
#ifndef CRYPT
    if (pInfo->encrypted) {
        fprintf(stderr, "   skipping: %-22s  encrypted (not supported)\n",
          filename);
        return 0;
    }
#endif /* !CRYPT */
    mapattr();   /* GRR:  worry about return value later */
    pInfo->offset = (long) crec.relative_offset_local_header;
    return 1;
} /* end function store_info() */
static int extract_or_test_member()    /* return PK-type error code */
{
    char *nul="[empty] ", *txt="[text]  ", *bin="[binary]";
    register int b;
    int r, error=PK_COOL;
    bits_left = 0;     /* unreduce and unshrink only */
    bitbuf = 0L;       /* unreduce and unshrink only */
    zipeof = 0;        /* unreduce and unshrink only */
    newfile = TRUE;
    crc32val = 0xFFFFFFFFL;
#ifdef SYMLINKS
    if (S_ISLNK(pInfo->file_attr) && (pInfo->hostnum == UNIX_) && !tflag &&
        !cflag && (lrec.ucsize > 0))
        symlnk = TRUE;
    else
        symlnk = FALSE;
#endif /* SYMLINKS */
    if (tflag) {
        if (!qflag) {
            fprintf(stdout, ExtractMsg, "testing", filename, "", "");
            fflush(stdout);
        }
    } else {
        if (cflag) {
            outfile = stdout;
#ifdef DOS_NT_OS2
            setmode(fileno(outfile), O_BINARY);
#endif
#ifdef VMS
            if (open_outfile())   /* VMS:  required even for stdout! */
                return PK_DISK;
#endif
        } else if (open_outfile())
            return PK_DISK;
    }
    switch (lrec.compression_method) {
        case STORED:
            if (!tflag && QCOND2) {
#ifdef SYMLINKS
                if (symlnk)   /* can also be deflated, but rarer... */
                    fprintf(stdout, ExtractMsg, "linking", filename, "", "");
                else
#endif /* SYMLINKS */
                fprintf(stdout, ExtractMsg, "extracting", filename,
                  (aflag != 1 /* && pInfo->textfile == pInfo->textmode */ )? ""
                  : (lrec.ucsize == 0L? nul : (pInfo->textfile? txt : bin)),
                  cflag? "\n" : "");
                fflush(stdout);
            }
            outptr = slide;
            outcnt = 0L;
            while ((b = NEXTBYTE) != EOF && !disk_full) {
                *outptr++ = (uch)b;
                if (++outcnt == WSIZE) {
                    flush(slide, outcnt, 0);
                    outptr = slide;
                    outcnt = 0L;
                }
            }
            if (outcnt)          /* flush final (partial) buffer */
                flush(slide, outcnt, 0);
            break;
#ifndef SFX
        case SHRUNK:
            if (!tflag && QCOND2) {
                fprintf(stdout, ExtractMsg, "unshrinking", filename,
                  (aflag != 1 /* && pInfo->textfile == pInfo->textmode */ )? ""
                  : (pInfo->textfile? txt : bin), cflag? "\n" : "");
                fflush(stdout);
            }
            if ((r = unshrink()) != PK_COOL) {
                if ((tflag && qflag) || (!tflag && !QCOND2))
                    fprintf(stderr,
                      "  error:  not enough memory to unshrink %s\n", filename);
                else
                    fprintf(stderr,
                      "\n  error:  not enough memory for unshrink operation\n");
                error = r;
            }
            break;
        case REDUCED1:
        case REDUCED2:
        case REDUCED3:
        case REDUCED4:
            if (!tflag && QCOND2) {
                fprintf(stdout, ExtractMsg, "unreducing", filename,
                  (aflag != 1 /* && pInfo->textfile == pInfo->textmode */ )? ""
                  : (pInfo->textfile? txt : bin), cflag? "\n" : "");
                fflush(stdout);
            }
            unreduce();
            break;
        case IMPLODED:
            if (!tflag && QCOND2) {
                fprintf(stdout, ExtractMsg, "exploding", filename,
                  (aflag != 1 /* && pInfo->textfile == pInfo->textmode */ )? ""
                  : (pInfo->textfile? txt : bin), cflag? "\n" : "");
                fflush(stdout);
            }
            if (((r = explode()) != 0) && (r != 5)) {   /* treat 5 specially */
                if ((tflag && qflag) || (!tflag && !QCOND2))
                    fprintf(stderr, "  error:  %s%s\n", r == 3?
                      "not enough memory to explode " :
                      "invalid compressed (imploded) data for ", filename);
                else
                    fprintf(stderr, "\n  error:  %s\n", r == 3?
                      "not enough memory for explode operation" :
                      "invalid compressed data for explode format");
                error = (r == 3)? PK_MEM2 : PK_ERR;
            }
            if (r == 5) {
                int warning = ((ulg)used_csize <= lrec.csize);
                if ((tflag && qflag) || (!tflag && !QCOND2))
                    fprintf(stderr, LengthMsg, "", warning? "warning":"error",
                      used_csize, lrec.ucsize, warning? "  ":"", lrec.csize,
                      " [", filename, "]");
                else
                    fprintf(stderr, LengthMsg, "\n", warning? "warning":"error",
                      used_csize, lrec.ucsize, warning? "  ":"", lrec.csize,
                      "", "", ".");
                error = warning? PK_WARN : PK_ERR;
            }
            break;
#endif /* !SFX */
        case DEFLATED:
            if (!tflag && QCOND2) {
                fprintf(stdout, ExtractMsg, "inflating", filename,
                  (aflag != 1 /* && pInfo->textfile == pInfo->textmode */ )? ""
                  : (pInfo->textfile? txt : bin), cflag? "\n" : "");
                fflush(stdout);
            }
            if ((r = inflate()) != 0) {
                if ((tflag && qflag) || (!tflag && !QCOND2))
                    fprintf(stderr, "  error:  %s%s\n", r == 3?
                      "not enough memory to inflate " :
                      "invalid compressed (deflated) data for ", filename);
                else
                    fprintf(stderr, "\n  error:  %s\n", r == 3?
                      "not enough memory for inflate operation" :
                      "invalid compressed data for inflate format");
                error = (r == 3)? PK_MEM2 : PK_ERR;
            }
            break;
        default:   /* should never get to this point */
            fprintf(stderr, "%s:  unknown compression method\n", filename);
            return PK_WARN;
    } /* end switch (compression method) */
    if (disk_full) {            /* set by flush() */
        if (disk_full > 1)
            return PK_DISK;
        error = PK_WARN;
    }
    if (!tflag && !cflag)   /* don't close NULL file or stdout */
        close_outfile();
    if (error > PK_WARN)  /* don't print redundant CRC error if error already */
        return error;
    if ((crc32val = ((~crc32val) & 0xFFFFFFFFL)) != lrec.crc32) {
        if ((tflag && qflag) || (!tflag && !QCOND2))
            fprintf(stderr, "%-22s ", filename);
        fprintf(stderr, " bad CRC %08lx  (should be %08lx)\n", crc32val,
                lrec.crc32);
        FFLUSH(stderr);
        error = PK_ERR;
    } else if (tflag) {
        if (!qflag)
            fprintf(stdout, " OK\n");
    } else {
        if (QCOND2 && !error)
            fprintf(stdout, "\n");
    }
    return error;
} /* end function extract_or_test_member() */
int memextract(tgt, tgtsize, src, srcsize)   /* extract compressed extra */
    uch *tgt, *src;                          /*  field block; return PK- */
    ulg tgtsize, srcsize;                    /*  type error level */
{
    uch *old_inptr=inptr;
    int  old_incnt=incnt, r, error=PK_OK;
    ush  method;
    ulg  extra_field_crc;
    method = makeword(src);
    extra_field_crc = makelong(src+2);
    inptr = src + 2 + 4;      /* method and extra_field_crc */
    incnt = (int)(csize = (long)(srcsize - (2 + 4)));
    mem_mode = TRUE;
    switch (method) {
        case STORED:
            memcpy((char *)tgt, (char *)inptr, (extent)incnt);
            outcnt = csize;   /* for CRC calculation */
            break;
        case DEFLATED:
            if ((r = inflate()) != 0) {
                fprintf(stderr, "error:  %s\n", r == 3 ?
                  "not enough memory for inflate operation" :
                  "invalid compressed data for the inflate format");
                error = (r == 3)? PK_MEM2 : PK_ERR;
            }
            if (outcnt == 0L)   /* inflate's final FLUSH sets outcnt */
                break;
            if (outcnt <= tgtsize)
                memcpy((char *)tgt, (char *)slide, (extent)outcnt);
            else
                error = PK_MEM3;   /* GRR:  should be passed up via SetEAs() */
            break;
        default:
            fprintf(stderr,
              "warning:  unsupported extra field compression type--skipping\n");
            error = PK_WARN;   /* GRR:  should be passed on up via SetEAs() */
            break;
    }
    inptr = old_inptr;
    incnt = old_incnt;
    mem_mode = FALSE;
    if (!error) {
        register ulg crcval = 0xFFFFFFFFL;
        register ulg n = outcnt;   /* or tgtsize?? */
        register uch *p = tgt;
        while (n--)
            crcval = crc_32_tab[((uch)crcval ^ (*p++)) & 0xff] ^ (crcval >> 8);
        crcval = (~crcval) & 0xFFFFFFFFL;
        if (crcval != extra_field_crc) {
            fprintf(stderr,
              "error [%s]:  bad extra field CRC %08lx (should be %08lx)\n",
              zipfn, crcval, extra_field_crc);
            error = PK_WARN;
        }
    }
    return error;
} /* end function memextract() */
