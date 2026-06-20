
major_version	    equ     7	    ;Major DOS version 7 Standard
;major_version	    equ     5	    ;Major DOS version 5 for LiberDOS
minor_version	    equ     00	    ;Minor DOS Version

expected_version    equ     (MINOR_VERSION SHL 8)+MAJOR_VERSION


;******************************
;Each assembler program should:
;  mov ah,030h			;DOS Get Version function
;  int 021h			;Version ret. in AX,minor version first
;  cmp ax,expected_version	;ALL utilities should check for an
;  jne error_handler		; EXACT version match.
;******************************

