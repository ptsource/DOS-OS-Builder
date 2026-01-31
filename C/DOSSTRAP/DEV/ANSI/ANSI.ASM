;********************************************************
;*                                                      *
;*               www.wiki.ptsource.eu                   *
;*                                                      *
;********************************************************

        extrn   f_escape:near, f_in_escape:near
        extrn   param_end:word
        extrn   dosfn0:near
        public  f_loopdone
        public  f_not_ansi
        public  f_ansi_exit
        public  cur_x, cur_y, max_x, cur_attrib
        public  xy_to_regs, get_blank_attrib
        public  port_6845
        public  wrap_flag
        public  cur_parm_ptr
        public  cur_coords, saved_coords, max_y
        public  escvector, string_term
        public  cpr_esc, cprseq
        public  video_mode
        public  req_ptr, break_handler
        public  int_29

        ; to all modules

keybuf  struc                           ; Used in getchar
len     dw      ?
adr     dw      ?
keybuf  ends


ABS40   segment at 40h
        org     1ah
buffer_head     dw      ?       ; Used in 'flush input buffer' dos call.
buffer_tail     dw      ?

        org     49h
crt_mode        db      ?
crt_cols        dw      ?
crt_len         dw      ?
crt_start       dw      ?
cursor_posn     dw      8 dup (?)
cursor_mode     dw      ?
active_page     db      ?
addr_6845       dw      ?
crt_mode_set    db      ?       ; = 7 only if monochrome display adaptor
crt_palette     db      ?
        org     6ch
timer_low       dw      ?       ; low word of time-of-day counter (18.2 hz)

ABS40   ends


CODE    segment byte public 'CODE'
assume  cs:code, ds:code

        ; Device Driver Header

        org     0

        dd      -1                      ; next device
        dw      8013h                   ; attributes
        dw      strategy                ; request header pointer entry
        dw      interrupt               ; request entry point
        db      'CON', 5 dup(' ')       ; device name (8 char)


;----- variable area --------------------
req_ptr label   dword
req_off dw      ?
req_seg dw      ?

wrap_flag       db      1       ; 0 = no wrap past line end
escvector       dw      0       ; state vector of ESCape sequencer
video_mode      db      3       ; ROM BIOS video mode (2=BW, 3=color)
max_y           db      24
max_cur_x       label   word    ; used to get both max & cur at once
max_x           db      79      ; line width (79 for 80x25 modes)
cur_coords      label   word
cur_x           db      0       ; cursor position (0 = left edge)
cur_y           db      0       ;                 (0 = top edge)
saved_coords    dw      ?       ; holds XY after a SCP escape sequence
string_term     db      0       ; either escape or double quote
cur_attrib      db      7       ; current char attributes
cur_page        db      0       ; current display page
video_seg       dw      ?       ; segment of video card
f_cptr_seg      dw      ?       ; part of fastout write buffer pointer
cur_parm_ptr    dw      ?       ; last byte of parm area now used
port_6845       dw      ?       ; port address of 6845 card

brkkeybuf       db      3       ; control C
fnkeybuf        db      ?       ; holds second byte of fn key codes
cpr_buf         db      8 dup (?), '['
cpr_esc         db      27      ; descending buffer for cpr function

; following four keybufs hold information about input
; Storage order determines priority- since the characters making up a function
; key code must never be separated (say, by a Control-Break), they have the
; highest priority, and so on.  Keyboard keys (except ctrl-break) have the
; lowest priority.

fnkey   keybuf  <0, fnkeybuf>   ; fn key string (0 followed by scan code)
cprseq  keybuf  <0>             ; CPR string (ESC [ y;x R)
brkkey  keybuf  <0, brkkeybuf>  ; ^C
xlatseq keybuf  <0>             ; keyboard reassignment string

;------ xy_to_regs --------------------------------------------
; on entry: x in cur_x, y in cur_y
; on exit:  dx = chars left on line, di = address
; Alters ax, bx.
xy_to_regs      proc    near
        ; Find number of chars til end of line, keep in DX
        mov     ax,max_cur_x
        mov     bl,ah
        xor     bh,bh
        cbw
        mov     dx,ax
        sub     dx,bx
        inc     dx                      ; DX is # of chars till EOL
        ; Calculate DI = current address in text buffer
        inc     ax                      ; AL = max_x
        mul     cur_y
        add     ax,bx                   ; AX is # of chars into buffer
        shl     ax,1
        mov     di,ax                   ; DI is now offset of cursor.
        ret
xy_to_regs      endp


;------- dos_fn_tab -------------
; This table is used in "interrupt" to call the routine that handles
; the requested function.

max_cmd equ     12
dos_fn_tab:
        dw      dosfn0, nopcmd, nopcmd, badcmd, dosfn4, dosfn5, dosfn6
        dw      dosfn7, dosfn8, dosfn8, nopcmd, nopcmd

;------- strategy ----------------------------------------------------
; DOS calls strategy with a request which is to be executed later.
; Strategy just saves the request.

strategy        proc    far
        mov     cs:req_off,BX
        mov     cs:req_seg,ES
        ret
strategy        endp

;------ interrupt -----------------------------------------------------
; This is where the request handed us during "strategy" is
; actually carried out.
; Calls one of 12 subroutines depending on the function requested.
; Each subroutine returns with exit status in AX.

interrupt       proc    far
        sti
        push    ax
        push    cx
        push    dx
        push    bx
        push    bp
        push    si
        push    di
        push    ds
        push    es

        ; Read requested function information into registers
        lds     bx,cs:req_ptr
        mov     al,2[BX]                ; al = function code
        les     si,14[BX]               ; ES:SI = input/output buffer addr
        mov     cx,18[BX]               ; cx = input/output byte count

        cmp     al,max_cmd
        ja      unk_command             ; too big, exit with error code

        mov     bx,ax
        shl     bx,1                    ; form index to table of words
        mov     ax,cs
        mov     ds,ax
        call    word ptr dos_fn_tab[bx]
int_done:
        lds     bx,cs:req_ptr           ; report status
        or      ax,100h                 ; (always set done bit upon exit)
        mov     3[bx],ax

        pop     ES                      ; restore caller's registers
        pop     DS
        pop     di
        pop     si
        pop     bp
        pop     bx
        pop     dx
        pop     cx
        pop     ax
        ret                             ; return to DOS.

unk_command:
        call    badcmd
        jmp     int_done

interrupt       endp

;----- BIOS break handler -----------------------------------------
; Called by BIOS when Control-Break is hit (vector was set up in Init).
; Simply notes that a break was hit.  Flag is checked during input calls.

break_handler   proc
        mov     cs:brkkey.len, 1
        iret
break_handler   endp


;------ badcmd -------------------------------------------------------
; Invalid function request by DOS.
badcmd  proc    near
        mov     ax, 813h                ; return "Error: invalid cmd"
        ret
badcmd  endp


;------ nopcmd -------------------------------------------------------
; Unimplemented or dummy function request by DOS.
nopcmd  proc    near
        xor     ax, ax                  ; No error, not busy.
        ret
nopcmd  endp

;------- dos function #4 -----------------------------------------------
; Reads CX characters from the keyboard, places them in buffer at ES:SI.
dosfn4  proc    near
        jcxz    dos4done
        mov     di,si
dos4lp: push    cx
        call    getchar
        pop     cx
        stosb
        loop    dos4lp
dos4done:
        xor     ax,ax                   ; No error, not busy.
        ret
dosfn4  endp

;-------- dos function #5: non-destructive input, no wait ------
; One-character lookahead into the keyboard buffer.
; If no characters in buffer, return BUSY; otherwise, get value of first
; character of buffer, stuff into request header, return DONE.
dosfn5  proc    near
        call    peekchar
        jz      dos5_busy
        lds     bx,req_ptr
        mov     [bx+0Dh], al
        xor     ax, ax                  ; No error, not busy.
        ret
dos5_busy:
        mov     ax, 200h                ; No error, busy.
        ret

dosfn5  endp

;-------- dos function #6: input status --------------------------
; Returns "busy" if no characters waiting to be read.
dosfn6  proc    near
        call    peekchar
        mov     ax, 200h                ; No error, busy.
        jz      dos6_exit
        xor     ax, ax                  ; No error, not busy.
dos6_exit:
        ret
dosfn6  endp

;-------- dos function #7: flush input buffer --------------------
; Clears the IBM keyboard input buffer.  Since it is a circular
; queue, we can do this without knowing the beginning and end
; of the buffer; all we need to do is set the tail of the queue
; equal to the head (as if we had read the entire queue contents).
; Also resets all the device driver's stuffahead buffers.
dosfn7  proc    near
        mov     ax, abs40
        mov     es, ax
        mov     ax, es:buffer_head      ; clear queue by making the tail
        mov     es:buffer_tail, ax      ; equal to the head
        xor     ax, ax                  ; no error, not busy
        mov     fnkey.len, ax           ; Reset the stuffahead buffers.
        mov     cprseq.len, ax
        mov     brkkey.len, ax
        mov     xlatseq.len, ax
        ret
dosfn7  endp

;------ int_29 ----------------------------------------------
; Int 29 handles DOS quick-access putchar.
; Last device loaded with attribute bit 4 set gets accessed for
; single-character writes via int 29h instead of via interrupt.
; Must preserve all registers.
; Installed as int 29h by dosfn0 (init).
int_29_buf      db      ?

int_29  proc    near
        sti
        push    ds
        push    es
        push    ax
        push    cx
        push    dx
        push    bx
        push    bp
        push    si
        push    di
        mov     cx,1
        mov     bx,cs
        mov     es,bx
        mov     ds,bx
        mov     si,offset int_29_buf
        mov     [si],al
        call    dosfn8
        pop     di
        pop     si
        pop     bp
        pop     bx
        pop     dx
        pop     cx
        pop     ax
        pop     es
        pop     ds
        iret
int_29  endp

;------ dosfn8 -------------------------------------------------------
; Handles writes to the device (with or without verify).
; Called with
;  CX    = number of bytes to write
;  ES:SI = transfer buffer
;  DS    = CS, so we can access local variables.

dosfn8  proc    near

        mov     f_cptr_seg, es  ; save segment of char ptr

        ; Read the BIOS buffer address/cursor position variables.
        mov     ax,abs40
        mov     ds,ax
        assume  ds:abs40

        ; Find current video mode and screen size.
        mov     ax,word ptr crt_mode    ; al = crt mode; ah = # of columns
        mov     cs:video_mode, al
        dec     ah                      ; ah = max column
        mov     cs:max_x, ah

        ; Find current cursor coordinates.
        mov     al,active_page
        cbw
        shl     ax,1
        mov     bx,ax
        mov     ax,cursor_posn[bx]
        mov     cs:cur_coords,AX

        ; Find video buffer segment address; adjust so ofs is 0; return in AX.
        ; DS is abs40.
        mov     ax,addr_6845            ; 6845 address
        mov     cs:port_6845,ax

        mov     ax,cs
        mov     ds,ax
        assume  ds:code

        mov     ax,0B000h               ; assume it's a monochrome card...
        cmp     video_mode,7
        jz      d8_gots
        mov     ah,0B8h                 ; but if not mode 7, it's color.
d8_gots:
        mov     video_seg,ax
        mov     es,ax
        call    xy_to_regs              ; Set DX, DI according to cur_coords.

        ; | If in graphics mode, clear old pseudocursor
        cmp     cs:video_mode, 4
        jb      d8_no_cp
        cmp     cs:video_mode, 7
        jz      d8_no_cp
        call    pseudocursor            ; write block in xor
d8_no_cp:
        mov     ah, cur_attrib
        mov     ds, f_cptr_seg          ; get segment of char ptr
        assume  ds:nothing
        cld                             ; make sure we'll increment

        ; Get a character, put it on the screen, repeat 'til end of line
        ; or no more characters.
        jcxz    f_loopdone              ; if count = 0, we're already done.
        cmp     cs:escvector, 0         ; If in middle of an escape sequence,
        jnz     f_in_escapex            ; jump to escape sequence handler.

f_tloop: ; If not in graphics mode, jump to alternate loop
         ; What a massive kludge!  A better approach would have been
         ; to collect characters for a "write n chars" routine
         ; which would handle both text and graphics modes.
        cmp     cs:video_mode,4
        jb      f_t_cloop
        cmp     cs:video_mode,7
        jz      f_t_cloop


f_g_cloop:
        lodsb                           ; get char! (al = ds:[si++])
        cmp     al,28                   ; is it a control char?
        jb      f_control               ;  maybe...
f_g_nctl:
        call    putchar
        dec     dx                      ; count down to end of line
        loopnz  f_g_cloop               ; and go back for more.
        jmp     short f_t_at_eol

f_t_cloop:
        lodsb                           ; get char! (al = ds:[si++])
        cmp     al,28                   ; is it a control char?
        jb      f_control               ;  maybe...
f_t_nctl:
        stosw                           ; Put Char! (es:[di++] = ax)
        dec     dx                      ; count down to end of line
        loopnz  f_t_cloop               ; and go back for more.
        jz      f_at_eol                ; at end of line; maybe do a crlf.
        jmp     short f_loopdone

f_looploop:
f_ansi_exit:                            ; in case we switched into
        loopnz  f_tloop                 ; a graphics mode
f_t_at_eol:
        jz      f_at_eol

f_loopdone:

        ;--------- All done with write request -----------
        ; DI is cursor address; cursor position in cur_y, dl.
        mov     ax, cs
        mov     ds, ax                  ; get our segment back
        assume  ds:code

        ; Restore cur_x = max_x - dx + 1.
        mov     al, max_x
        inc     al
        sub     al, dl
        mov     cur_x, al
        ; Set cursor position; cursor adr in DI; cursor pos in cur_x,cur_y
        call    set_pseudocursor
        ; Return to DOS.
        xor     ax, ax                  ; No error, not busy.
        ret

        ;---- handle control characters ----
        ; Note: cur_x is not kept updated in memory, but can be
        ; computed from max_x and dx.
        ; Cur_y is kept updated in memory.
f_control:
        cmp     al,13                   ; carriage return?
        jz      f_cr
        cmp     al,10                   ; line feed?
        jz      f_lf
        cmp     al,27                   ; Is it an escape?
        jz      f_escapex
        cmp     al,8                    ; backspace?
        jz      f_bs
        cmp     al,9                    ; tab?
        jz      f_tabx
        cmp     al,7                    ; bell?
        jz      f_bell
f_not_ansi:                             ; not a control char
        cmp     cs:video_mode, 4
        jb      f_t_nctl
        cmp     cs:video_mode, 7
        jz      f_t_nctl
        jmp     f_g_nctl

f_tabx: jmp     f_tab
f_escapex:
        jmp     f_escape
f_in_escapex:
        jmp     f_in_escape

f_bs:   ;----- Handle backspace -----------------
        ; Moves cursor back one space without erasing.  No wraparound.
        cmp     dl, cs:max_x            ; wrap around to previous line?
        ja      fbs_wrap                ; yep; disallow it.
        dec     di                      ; back up one char & attrib
        dec     di
        inc     dx                      ; and note one more char left on line.
fbs_wrap:
        jmp     f_looploop

f_bell: ;----- Handle bell ----------------------
        call    beep
        or      al, al                  ; clear z
        jmp     f_looploop              ; Let main loop decrement cx.

f_cr:   ;----- Handle carriage return -----------
        ; di -= cur_x<<1;               set di= address of start of line
        ; dx=max_x+1;                   set bx= chars left in line
        mov     al, cs:max_x
        cbw
        inc     ax
        sub     al,dl                   ; Get cur_x into ax.
        sub     di,ax
        sub     di,ax
        mov     dl,cs:max_x             ; Full line ahead of us.
        inc     dx
        mov     ah,cs:cur_attrib        ; restore current attribute
        or      al,1                    ; clear z
        jmp     f_looploop              ; and let main loop decrement cx

f_at_eol:
        ;----- Handle overrunning right end of screen -------
        ; cx++;                         compensate for double loop
        ; if (!wrap_flag) { dx++; di-=2; }
        ; else do_crlf;
        inc     cx
        test    cs:wrap_flag, 1
        jnz     feol_wrap
        dec     di
        dec     di
        inc     dx
        jmp     f_looploop
feol_wrap:
        ; dx=max_x+1;                   set bx= chars left in line
        ; di -= 2*(max_x+1);
        ; do_lf
        mov     dl, cs:max_x
        inc     dx
        sub     di, dx
        sub     di, dx
        ; fall thru to line feed routine

f_lf:   ;----- Handle line feed -----------------
        ; if (cur_y >= max_y) scroll;           scroll screen up if needed
        ; else { cur_y++; di += max_x<<1;       else increment Y

        mov     al,cs:Max_y
        cmp     cs:Cur_y,al             ; do we need to scroll screen?
        jae     flf_scroll              ;   yes, do it
        inc     cs:cur_y
        mov     al,cs:Max_x
        cbw
        inc     ax
        shl     ax,1
        add     di,ax
        mov     ah,cs:cur_attrib        ; restore current attribute
        or      al,1                    ; clear z
        jmp     f_looploop              ; and let main loop decrement cx
flf_scroll:
        push    ax
        push    bx
        push    cx
        push    dx
        call    get_blank_attrib
        mov     bh,ah                   ; color to use on new blank areas
        mov     dl,cs:max_x
        mov     dh,cs:max_y
        cmp     cs:video_mode,4
        jb      flf_scrollt
        cmp     cs:video_mode,7
        je      flf_scrollt
        mov     al,1                    ; AL is number of lines to scroll.
        mov     ah,6                    ; BIOS: scroll up
        xor     cx,cx
        int     10h                     ; call BIOS to scroll a rectangle.
flf_scroll_done:
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        mov     ah,cs:cur_attrib        ; restore current attribute
        or      al,1                    ; clear z
        jmp     f_looploop              ; and let main loop decrement cx
flf_scrollt:
        push    es
        push    ds
        push    si
        push    di
        mov     ax,abs40
        mov     ds,ax
        assume  ds:abs40
        mov     al,active_page
        cbw
        mov     cx,ax
        xor     di,di                   ; starting video loc
        mov     ax,0B000h               ; assume mono
        cmp     cs:video_mode,7
        je      flf_st_got
        mov     ah,0B8h                 ; it's color
        jcxz    flf_st_got
flf_st_getpage:
        add     di,4096                 ; move to active page
        loop    flf_st_getpage
flf_st_got:
        mov     ds,ax                   ; set DS and ES to video segment
        mov     es,ax
        mov     si,di
        mov     al,dh
        inc     dx
        mul     dl
        mov     cx,ax                   ; CX is number of chars to move
        xor     dh,dh
        add     si,dx
        add     si,dx                   ; SI points to one line below DI
        rep     movsw                   ; scroll it
        mov     cx,dx                   ; CX is chars per line
        mov     ah,bh                   ; attribute
        mov     al," "
        rep     stosw                   ; clear the bottom line
        pop     di
        pop     si
        pop     ds
        pop     es
        assume  ds:code
        jmp     flf_scroll_done


f_tab:  ;----- Handle tab expansion -------------
        ; Get cur_x into al.
        mov     al, cs:max_x
        inc     al
        sub     al, dl
        ; Calculate number of spaces to output.
        push    cx                      ; save cx
        mov     cl, al                  ; get zero based x coordinate
        and     cx,7
        neg     cl
        add     cl,8                    ; 0 -> 8, 1 -> 8, ... 7 -> 1
        sub     dx, cx                  ; update chars-to-eol, maybe set z
        pushf                           ; || save Z for main loop
        ; ah is still current attribute.  Move CX spaces to the screen.
        mov     al, ' '
        cmp     cs:video_mode, 4
        jb      F_SPC_MV
        cmp     cs:video_mode, 7
        jnz     f_tab_putc
F_SPC_MV:
        REP     STOSW
        popf                            ; || restore Z flag for main loop test
        pop     cx                      ; restore cx
        jmp     f_looploop              ; Let main loop decrement cx.

;--------------- graphics mode support -----------------------

f_tab_putc:     ; graphics mode- call putc to put the char
        add     dx, cx                  ; move back to start of tab
f_tp_lp:
        call    putchar
        dec     dx                      ; go to next cursor position
        loop    f_tp_lp
        popf                            ; Z set if wrapped around EOL
        pop     cx
        jmp     f_looploop


;---- putchar ------------------------------------------------
; Writes char AL, attribute AH to screen at (max_x+1-dl), cur_y.
; On entry, registers set up as per xy_to_regs.
; Preserves all registers.
putchar proc    near
        push    dx
        push    cx
        push    bx
        push    ax
        ; 1. Set cursor position.
        mov     al, cs:max_x
        inc     al
        sub     al,dl
        mov     cs:cur_x, al
        mov     dx, cs:cur_coords       ; get X & Y into DX
        xor     bx, bx                  ; choose dpy page 0
        mov     ah, 2                   ; chose "Set Cursor Position"
        int     10h                     ; call ROM BIOS
        ; 2. Write char & attribute.
        mov     cx,1
        pop     ax                      ; get char in AL
        push    ax
        mov     bl,ah                   ; attribute in BL
        xor     bh,bh
        mov     ah,9
        int     10h
        pop     ax
        pop     bx
        pop     cx
        pop     dx
        ret
putchar endp

;---- set_pseudocursor ------------
; If in graphics mode, set pseudocursor, else set real cursor.
; Destroys DS!!!!

set_pseudocursor        proc    near
        cmp     cs:video_mode, 4
        jb      SET_CURS
        cmp     cs:video_mode, 7
        jnz     pseudocursor

SET_CURS:    ; Write directly to 6845 cursor address register.
        mov     bx,di
        shr     bx,1                   ; convert word index to byte index

        mov     dx,Port_6845
        mov     al,0Eh
        out     dx,al

        jmp     $+2
        inc     dx
        mov     al, bh
        out     dx, al

        jmp     $+2
        dec     dx
        mov     al, 0fh
        out     dx, al

        jmp     $+2
        inc     dx
        mov     al, bl
        out     dx, al

        ; Set cursor position in low memory.
        assume  ds:abs40
        mov     ax, abs40
        mov     ds, ax
        mov     ax, cs:cur_coords
        mov     cursor_posn,ax
        ret

        assume  ds:code
set_pseudocursor        endp


;---- pseudocursor --------------------------------------------------
; Writes a color 15 block in XOR at the current cursor location.
; Preserves DS, ES, BX, CX, DX, SI, DI.
; Should be disableable- the pseudocursor slows down single-char
; writes by a factor of three.
pseudocursor    proc    near
        mov     ax, 8f16h       ; xor, color 15, ^V (small block)
        call    putchar
        ret
pseudocursor    endp

;--------------- end of graphics mode support --------------------

dosfn8  endp

;--- get_blank_attrib ------------------------------------------------
; Determine new attribute and character for a new blank region.
; Use current attribute, just disallow blink and underline.
; (Pretty strange way to do it.  Might want to disallow rev vid, too.)
; Returns result in AH, preserves all other registers.
get_blank_attrib        proc    near
        xor     ah,ah
        cmp     cs:video_mode, 4
        jb      GB_TX
        cmp     cs:video_mode, 7
        jnz     gb_aok                  ; if graphics mode, 0 is bkgnd
GB_TX:
        mov     ah, cs:cur_attrib
        and     ah,7fh                  ; disallow blink
        cmp     cs:video_mode,7         ; monochrome?
        jne     gb_aok
        cmp     ah,1                    ; underline?
        jne     gb_aok
        mov     ah,7                    ; yep- set it to normal.
gb_aok: ret
get_blank_attrib        endp


;---- searchbuf --------------------------------------------
; Called by getchar and peekchar to see if any characters are
; waiting to be gotten from sources other than BIOS.
; Returns with Z set if no chars found, BX=keybuf & SI=keybuf.len otherwise.
searchbuf       proc    near
        ; Search the stuffahead buffers.
        mov     cx,4                    ; number of buffers to check for chars
        mov     bx,offset fnkey -4
sbloop: add     bx,4                    ; point to next buffer record
        mov     si,[bx].len
        or      si,si                   ; empty?
        loopz   sbloop                  ; if so, loop.
        ret
searchbuf       endp

;---- getchar -----------------------------------------------
; Returns AL = next char.
; Trashes AX, BX, CX, BP, SI.
getchar proc    near
gc_searchbuf:
        ; See if any chars are waiting in stuffahead buffers.
        call    searchbuf
        jz      gc_trykbd               ; No chars?  Try the keyboard.
        ; A nonempty buffer was found.
        dec     [bx].len
        dec     si
        mov     bp, [bx].adr            ; get pointer to string
        mov     al, byte ptr ds:[bp][si]; get the char
        ; Recognize function key sequences, move them to highest priority
        ; queue.
        sub     si,1                    ; set carry if si=0
        jc      gc_nofnkey              ; no chars left -> nothing to protect.
        cmp     bx, offset fnkey
        jz      gc_nofnkey              ; already highest priority -> done.
        or      al,al
        jnz     gc_nofnkey              ; nonzero first byte -> not fnkey.
        ; Found a function key; move it to highest priority queue.
        dec     [bx].len
        mov     ah, byte ptr ds:[bp][si]; get the second byte of fn key code
gc_fnkey:
        mov     fnkey.len, 1
        mov     fnkeybuf, ah            ; save it.
gc_nofnkey:
        ret                             ; Valid char in AL.  Return with it.

gc_trykbd:
        xor     ah,ah
        int     16h                     ; BIOS returns with char in AX
        ; If it's Ctrl-break, it has already been taken care of.
        or      ax, ax
        jz      gc_trykbd

gcbark: or      al,al                   ; Is it a function key?
        jz      gc_fnkey                ;   yep, special treatment
gcdone: ret     ; with character in AL.
getchar endp

;---- peekchar -----------------------------------------------
; Returns Z if no character ready, AL=char otherwise.
; Trashes AX, BX, CX, BP, SI.
peekchar        proc    near
pc_searchbuf:
        call    searchbuf
        jz      pc_trykbd               ; No chars?  Try the keyboard.
        ; A nonempty buffer was found.
        dec     si
        mov     bp,[bx].adr             ; get pointer to string
        mov     al,byte ptr ds:[bp][si] ; get the char
        ; Valid char from buffer in AL.  Return with it.
        jmp     short pcdone

pc_brk: int     16h                     ; get rid of control-break in buffer

pc_trykbd:
        mov     ah,1
        int     16h                     ; BIOS returns with char in AX
        jz      pcexit
        or      ax,ax
        jz      pc_brk  ; If ctl-brk, it's already been taken care of- kill it.

pcdone: or      ah,1                    ; NZ; char ready!
pcexit: ret     ; with character in AL, Z true if no char waiting.
peekchar        endp

;---- beep ------------------------------------------------------
; Beep speaker; period given by beep_div, duration by beep_len.
; Preserves all registers.

beep_div        dw      1300            ; fairly close to IBM beep
beep_len        dw      3               ; 3/18 sec- shorter than IBM

beep    proc    near
        push    ax
        push    cx
        push    dx
        push    bx
        push    bp
        push    si
        push    di

        mov     al,10110110b            ; select 8253
        mov     dx,43h                  ; control port address
        out     dx,al
        dec     dx                      ; timer 2 address
        mov     ax, cs:beep_div
        out     dx,al                   ; low byte of divisor
        mov     al,ah
        out     dx,al                   ; high byte of divisor
        mov     dx,61h
        in      al,dx                   ; get current value of control bits
        push    ax
        or      al, 3
        out     dx,al                   ; turn speaker on

        ; Wait for desired duration by monitoring time-of-day 18 Hz clock
        push    es
        mov     ax,abs40
        mov     es,ax
        assume  es:abs40
        mov     bx,timer_low
        add     bx,cs:beep_len
        mov     cx, -1                  ; emergency, in case clock dead

beeplp: mov     ax, timer_low
        cmp     ax,bx
        loopne  beeplp

        pop     es
        assume  es:code
        pop     ax
        and     al,0FCh                 ; turn speaker off
        out     dx,al
        pop     di
        pop     si
        pop     bp
        pop     bx
        pop     dx
        pop     cx
        pop     ax
        ret
beep    endp

CODE    ends
        end
