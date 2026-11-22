;
; stristr, search string without upper/lowercase check!
; combination version of strupr and strstr from Pure C.
;
; Patched by my (Koos Kuil) on 25 September 1994
;
; char *stristr (const char *s1, const char *s2);
;
; s1 = string
; s2 = search-string (must be uppercase!)
;

xdef _UpcTab

xdef stristr
stristr:

    move.l    a2, -(a7)
    move.l    a3, -(a7)
    move.l    a4, -(a7)
   
    move.b    (a1)+, d0
    beq        x2
    movea.l    a1, a2

    lea.l     _UpcTab (pc), a4      ; Upper case mask

    clr.w    d1

again:
   
    move.b    (a0)+, d1
    move.b    (a4, d1.w), d1
   
    beq        x2
    cmp.b    d0, d1
    bne.b    again
   
    movea.l    a0, a3
   
again2:
   
    move.b     (a1)+, d2

    beq.b    x1

    move.b    (a0)+, d1
    move.b  (a4, d1.w), d1
       
    beq.b    x2
    cmp.b    d1, d2
   
    beq.b    again2
   
    movea.l    a2, a1
    movea.l    a3, a0
    bra.b    again
   
x1:

    lea.l    -$0001 (a3), a0

end:

    movea.l (a7)+, a4
    movea.l (a7)+, a3
    movea.l (a7)+, a2
    rts
   
x2:

    suba.l     a0, a0
    bra        end

; End  stristr.s
