; Author       : Avni Agarwal
; Roll No.     : 2501AI27
; Test 3: every kind of error (all reported in one run)
start:  ldc 1
        ldc nonesuch    ; undefined label
        ldc 08ge        ; bad operand
dup:    ldc 5
dup:    ldc 6           ; duplicate label
        br missing_label
        foo 1           ; unknown mnemonic
        ldc             ; missing operand
        add 4           ; unexpected operand
        ldc 99999999    ; out of 24-bit range
value:  data xyz        ; invalid DATA value
        HALT
