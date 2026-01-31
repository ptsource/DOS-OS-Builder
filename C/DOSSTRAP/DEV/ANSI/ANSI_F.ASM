;********************************************************
;*                                                      *
;*               www.wiki.ptsource.eu                   *
;*                                                      *
;********************************************************


        public  ansi_fn_table
        extrn   port_6845:word
        extrn   cur_coords:word, saved_coords:word
        extrn   cur_x:byte, max_x:byte
        extrn   cur_y:byte, max_y:byte
        extrn   cur_attrib:byte, wrap_flag:byte
        extrn   xy_to_regs:near
        extrn   get_blank_attrib:near
        extrn   cpr_esc:byte, cprseq:word
        extrn   video_mode:byte

keybuf  struc                           ; used in making cpr sequence
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
crt_mode_set    db      ?
crt_palette     db      ?

ABS40   ends

code    segment byte public 'CODE'
        assume cs:code, ds:code

;----- byteout ---------------------------------------------------
; Converts al to a decimal ASCII string (in 0..99),
; stores it at ES:DI++.  Returns DI pointing at byte after last digit.
; Destroys DL.

byteout proc    near
        aam
        add     ax, 3030h
        xchg    ah, al
        stosb
        xchg    ah, al
        stosb
        ret
byteout endp

;----- ansi_fn_table -----------------------------------
; Table of offsets of terminal control subroutines in order of
; the character that invokes them, @..Z, a..z.  Exactly 53 entries.
; All the subroutines are defined below in this module.
ansi_fn_table   label   word
        dw      ic,  cup, cdn, cfw, cbk         ; @, A, B, C, D
        dw      nul, nul, nul, hvp, nul         ; E, F, G, H, I
        dw      eid, eil, il,  d_l, nul         ; J, K, L, M, N
        dw      nul, dc,  nul, nul, nul         ; O, P, Q, R, S
        dw      nul, nul, nul, nul, nul         ; T, U, V, W, X
        dw      nul, nul                        ; Y, Z
        dw      nul, nul, nul, nul, nul         ; a, b, c, d, e
        dw      hvp, nul, sm,  nul, nul         ; f, g, h, i, j
        dw      nul, rm,  sgr, dsr, nul         ; k, l, m, n, o
        dw      nul, nul, nul, scp, nul         ; p, q, r, s, t
        dw      rcp, nul, nul, nul, nul         ; u, v, w, x, y
        dw      nul                             ; z

ansi_functions  proc    near            ; set return type to NEAR

;----- nul ---------------------------------------------
; No-action ansi sequence; called when unknown command given.
nul:    ret

;----- Cursor Motion -----------------------------------------------

;-- cursor to y,x
hvp:    or      al,al           ; First parameter is desired Y coordinate.
        jz      hvp_yok
        dec     ax              ; Convert to zero-based coordinates.
hvp_yok:mov     cur_y,al
        ; Get second parameter, if it is there, and set X with it.
        xor     ax,ax
        cmp     cx,2            ; was there a second parameter?
        jb      hvp_xok
        lodsb                   ; yes.
        or      al,al
        jz      hvp_xok
        dec     ax              ; convert to zero-based coordinates.
hvp_xok:mov     cur_x,al

        ; Clip to maximum coordinates.
hvp_set:
        mov     ax, cur_coords          ; al = x, ah = y
        cmp     al, max_x
        jbe     hvp_sxok
        mov     al, max_x
        mov     cur_x, al
hvp_sxok:
        cmp     ah, max_y
        jbe     hvp_syok
        mov     al, max_y
        mov     cur_y, al
hvp_syok:
        ; Set values of DX and DI accordingly.
        call    xy_to_regs
        ret

;-- cursor forward --
cfw:    add     cur_x, al
        jmp     hvp_set

;-- cursor back -----
cbk:    sub     cur_x, al
        jae     cbk_ok
        mov     cur_x, 0
cbk_ok: jmp     hvp_set

;-- cursor down -----
cdn:    add     cur_y, al
        jmp     hvp_set

;-- cursor up -------
cup:    sub     cur_y, al
        jae     cup_ok
        mov     cur_y, 0
cup_ok: jmp     hvp_set

;-- save cursor position --------------------------------------
scp:    mov     ax, cur_coords
        mov     saved_coords, ax
        ret

;-- restore cursor position -----------------------------------
rcp:    mov     ax, saved_coords
        mov     cur_coords, ax
        jmp     hvp_set         ; Clip in case we have switched video modes.

;-- set graphics rendition ------------------------------------
; Modifies the color in which new characters are written.

sgr:    dec     si              ; get back pointer to first parameter
        or      cx, cx          ; Did he give any parameters?
        jnz     sgr_loop
        mov     [si],cl         ; no parameters, so fake
        inc     cx              ; one with the default value.

sgr_loop:                ; Handles each parameter
        lodsb                   ; al = next parameter
        push    cx
        mov     cx,colors
        mov     bx,offset color_table -3
sgr_search:
        add     bx,3
        cmp     al,[bx]
        loopne  sgr_search      ; until match found or done
        jne     sgr_loopx

        ; If parameter named a known color, set the current color variable.
        mov     bx,1[bx]
        mov     al,cur_attrib
        and     al,bl
        or      al,bh
        mov     cur_attrib,al
sgr_loopx:
        pop     cx
        loop    sgr_loop                ; until no more parameters.
        ret

;-- erase in line ----------------------------------------
; Uses BIOS to scroll away a one-line rectangle
eil:    push    dx
        mov     cx, cur_coords
        mov     dh, ch
        jmp     short scrollem

;-- erase in display -------------------------------------
; Uses BIOS to scroll away all of display
eid:    cmp     al,2
        jnz     eid_ignore      ; param must be two
        xor     cx,cx
        mov     cur_coords,cx
        call    xy_to_regs      ; doesn't modify CX
        push    dx
        mov     dh,max_y
scrollem:
        call    get_blank_attrib
        mov     bh,ah
        mov     dl,max_x
        mov     ax,600h
        int     10h
        pop     dx
eid_ignore:
        ret

;-- device status report --------------------------------
; Stuffs an escape, a left bracket, current Y, semicolon, current X,
; a capital R, and a carriage return into input stream.
; The coordinates are 1 to 3 decimal digits each.

dsr:    push    di
        push    dx
        push    es
        mov     ax,cs
        mov     es,ax
        std                     ; Store string in reversed order for fun
        mov     di,offset cpr_esc - 2
        mov     al,cur_y
        inc     al              ; convert to one-based coords
        call    byteout         ; row
        mov     al,';'
        stosb
        mov     al,cur_x
        inc     al              ; convert to one-based coords
        call    byteout         ; column
        mov     al,'R'          ; R ANSI function 'Cursor Position Report'
        stosb
        mov     al,13
        mov     word ptr cprseq.adr, di ; save pointer to last char in string
        stosb                           ; send a carriage return, too
        mov     ax,offset cpr_esc
        sub     ax,di                   ; ax is # of characters in string
        mov     word ptr cprseq.len,ax  ; pass info to the getchar routine
        cld
        pop     es
        pop     dx
        pop     di
        ret

;---- Delete/Insert Lines -------------------------------
; AL is number of lines to delete/insert.
; Preserves DX, DI; does not move cursor.

d_l:    ; Delete lines.
        mov     ah, 6                   ; BIOS: scroll up
        jmp     short il_open

il:     ; Insert lines.
        mov     ah, 7                   ; BIOS: scroll down

il_open:
        ; Whether inserting or deleting, limit him to (max_y - cur_y) lines;
        ; if above that, we're just clearing; set AL=0 so BIOS doesn't burp.
        mov     bh, max_y
        sub     bh, cur_y
        cmp     al, bh
        jbe     il_ok                   ; DRK 9/4...
        xor     al,al           ; he tried to move too far
il_ok:
        push    ax
        call    get_blank_attrib
        mov     bh, ah                  ; color to use on new blank areas
        pop     ax                      ; AL is number of lines to scroll.

        xor     cl,cl                   ; upper-left-x of data to scroll
        mov     ch, cur_y               ; upper-left-y of data to scroll
        push    dx
        mov     dl, max_x               ; lower-right-x
        mov     dh, max_y               ; lower-right-y (zero based)
        int     10h                     ; call BIOS to scroll a rectangle.
        pop     dx
        ret                             ; done.

;-- Insert / Delete Characters ----------------------------
; AL is number of characters to insert or delete.
; Preserves DX, DI; does not move cursor.

ic:     mov     ch, 1                   ; 1 => swap dest & source below
        jmp     short dc_ch

dc:     xor     cl,cl

dc_ch:
        cmp     cs:video_mode, 4
        jb      DC_CH_DO
        cmp     cs:video_mode, 7
        jnz     dc_ret                  ; | if in graphics mode, ignore.
DC_CH_DO:
        ; AL = number of chars to ins or del (guarenteed nonzero).
        ; Limit him to # of chars left on line.
        cmp     al, dl
        jbe     dc_cok
        mov     al, dl
dc_cok:
        push    di                      ; DI is current address of cursor
        xchg    ax, cx                  ; CX gets # of chars to ins/del
        mov     bp, cx                  ; BP gets # of columns to clear.

        ; Set up source = destination + cx*2, count = dx - cx
        xor     ch,ch                   ; make it a word
        mov     si, di
        add     si, cx
        add     si, cx
        neg     cl
        add     cl, dl
        xor     ch,ch                   ; CX = # of words to transfer
        cld                             ; REP increments si & di

        ; If this is an insert, then flip transfer around in both ways.
        test    ah, 1
        jz      dc_noswap
        xchg    di,si           ; source <-> dest
        std                     ; up <-> down
        mov     ax,cx           ; make move over same range
        dec     ax
        shl     ax,1            ; AX=dist from 1st to last byte.
        add     di,ax           ; Start transfer at high end of block
        add     si,ax           ;  instead of low end.
dc_noswap:
        ; Move those characters.
        push    es
        pop     ds
        rep     movsw
        mov     cx,bp
        ; Figure out what color to make the new blanks.
        call    get_blank_attrib
        mov     al,' '
        ; Blank out vacated region.
        rep     stosw

        ; All done.
        cld                             ; restore normal REP state and
        pop     di                      ;  cursor address.
dc_ret: ret


;---- set / reset mode ---------------------------------------
; Sets graphics/text mode; also sets/resets "no wrap at eol" mode.
sm:     mov     cl, 0ffh        ; set
sm_rs:
        ; Is it "wrap at eol" ?
        cmp     al,7
        jnz     sm_notwrap
        mov     wrap_flag, cl   ; true = wrap at EOL
        ret
sm_notwrap:
        ; Is it "set highest number of screen lines available"?
        cmp     al, 43
        jnz     sm_video
        ; Only valid for the Enhanced Graphics Adaptor on
        ; a monochrome display or an enhanced color display.
        ; Test presence of EGA by calling BIOS fn 12h.10h.
        mov     ah, 12h
        mov     bx, 0ff10h
        int     10h                     ; bh=0-1, bl=0-3 if EGA
        test    bx, 0FEFCH
        jnz     sm_done                 ; sorry, charlie
        ; 43 line mode only allowed in text modes, for now.
        cmp     cs:video_mode, 4
        jb      DO_43L
        cmp     cs:video_mode, 7
        jnz     sm_done

DO_43L:
        xor     ah,ah                   ; "Set video mode"
        mov     al, video_mode          ; Re-init current mode
        int     10h

        mov     ax,1112h                ; Load 8x8 font
        xor     bl,bl                   ; (instead of 8x14)
        int     10h

        mov     ax, 1200h               ; Load new printscreen
        mov     bl,20h
        int     10h

        mov     ah,1
        mov     cx,0707h                ; (Load cursor scan lines)
        int     10h
        ; | Patch; this gotten by painful observation of
        ; | IBM's professional editor.  I think there's a
        ; | documented bug in Video Bios's "load cursor scan line"
        ; | call; try looking in latter 1985 PC Tech Journal.
        mov     dx, port_6845           ; '6845' command reg
        mov     al, 10
        out     dx, al
        inc     dx
        mov     al, 7
        out     dx, al                  ; set cursor start line
        ; Assume that gets us 43 lines.
        mov     max_y, 42
        jmp     short sm_home
sm_video:
        ; It must be a video mode.  Call BIOS.
        xor     ah,ah           ; "set video mode"
        int     10h
        ; Assume that gets us 25 lines.
        mov     max_y,24
sm_home:
        ; Read the BIOS buffer address/cursor position variables.
        mov     ax,abs40
        push    ds
        mov     ds,ax
        assume  ds:abs40
        ; Find current video mode and screen size.
        mov     ax,word ptr crt_mode    ; al = crt mode; ah = # of columns
        pop     ds
        mov     video_mode,al
        dec     ah                      ; ah = max column
        mov     max_x,ah

        ; Since cursor may end up in illegal position, it's best to
        ; just go home after switching video modes.
        mov     cur_coords,0
        call    xy_to_regs
sm_done:
        ret

rm:     xor     cl,cl           ; reset
        jmp     sm_rs


;********************************************************
;*                                                      *
;*               www.wiki.ptsource.eu                   *
;*                                                      *
;********************************************************

ansi_functions  endp    ; end dummy procedure block



;-------- Color table -----------------------------------------
; Used in "set graphics rendition"
colors  equ     22                      ; number of colors in table
color_table:
        db      0, 000h,07h             ; all attribs off; normal.
        db      1, 0ffh,08h             ; bold
        db      4, 0f8h,01h             ; underline
        db      5, 0ffh,80h             ; blink
        db      7, 0f8h,70h             ; reverse
        db      8, 088h,00h             ; invisible

        db      30,0f8h,00h             ; black foreground
        db      31,0f8h,04h             ; red
        db      32,0f8h,02h             ; green
        db      33,0f8h,06h             ; yellow
        db      34,0f8h,01h             ; blue
        db      35,0f8h,05h             ; magenta
        db      36,0f8h,03h             ; cyan
        db      37,0f8h,07h             ; white

        db      40,08fh,00h             ; black background
        db      41,08fh,40h             ; red
        db      42,08fh,20h             ; green
        db      43,08fh,60h             ; yellow
        db      44,08fh,10h             ; blue
        db      45,08fh,50h             ; magenta
        db      46,08fh,30h             ; cyan
        db      47,08fh,70h             ; white


code    ends
        end
