; Test 1: your sample program - all number forms, forward/backward labels
label:  ldc 0
        ldc -5
        ldc +5
loop:   br loop        ; backward/self jump -> warning
        br next
next:   ldc loop
        ldc var1
var1:   data 0
        HALT
