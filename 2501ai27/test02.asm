; Author       : Avni Agarwal
; Roll No.     : 2501AI27
; Test 2: mul and div, plus decimal / hex / octal operands
        ldc 6
        ldc 0x07       ; hex
        mul
        ldc 010        ; octal = 8
        ldc -0x10      ; negative hex
        div
        adj -3
        a2sp
        sp2a
        HALT
