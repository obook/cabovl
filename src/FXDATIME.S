;Ne sert pas, mais c'est bon exemple : 
 
             
.TEXT 
 
    XDEF    Fxdatime 
 
; Application : VOID Fdatime ( DOSTIME *timeptr, WORD handle, WORD wflag );

Fxdatime:

    move.w  d0,-(sp)            ; wflag   ( data, word) 
    move.w  d0,-(sp)            ; handle  ( data, word) 
    move.l  a0,-(sp)            ; timeptr ( pointeur) 
    move.w  #87,-(sp) 
    trap    #1 
    add.l   #$A,sp 
;   lea     $14(sp),sp 
    rts 
 
.END 
