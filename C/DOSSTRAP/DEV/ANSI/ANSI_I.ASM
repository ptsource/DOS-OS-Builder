;********************************************************
;*                                                      *
;*               www.wiki.ptsource.eu                   *
;*                                                      *
;********************************************************

        public  dosfn0
        extrn   break_handler:near
        extrn   int_29:near
        extrn   req_ptr:dword
        extrn   param_buffer:word       ; adr of first byte free for params
        extrn   param_end:word          ; adr of last byte used for params


code    segment byte public 'CODE'
        assume  cs:code, ds:code

;-------- dos function # 0 : init driver ---------------------
; Initializes device driver interrupts and buffers, then
; passes ending address of the device driver to DOS.
; Since this code is only used once, the buffer can be set up on top
; of it to save RAM.

dosfn0  proc    near
        ; Install BIOS keyboard break handler.
        xor     ax,ax
        mov     ds,ax
        mov     bx,6Ch
        mov     word ptr [BX],offset break_handler
        mov     [BX+02],cs
        ; Install INT 29 quick putchar.
        mov     bx,0a4h
        mov     word ptr [bx],offset int_29
        mov     [bx+2],cs

        push    cs
        pop     ds
        push    cs
        pop     es                      ; es=cs so we can use stosb
        cld                             ; make sure stosb increments di

        ; Calculate addresses of start and end of parameter buffer.
        ; The buffer occupies the same area of memory as this code!
        ; ANSI parameters are accumulated at the lower end, and
        ; param_end is the last byte used by params
        mov     di,offset dosfn0
        mov     param_buffer,di
        add     di,512
        mov     param_end,di    ; addr of last byte in free area
        inc     di

        ; Return ending address of this device driver, with status in AX.
        lds     si,req_ptr
        mov     14[si],di
        mov     16[si],cs

        xor     ax,ax              ; Return exit status in ax.
        ret

dosfn0  endp

code    ends
        end                            
