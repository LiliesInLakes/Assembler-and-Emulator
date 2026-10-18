; Test 5: warnings only (still assembles) - unused labels, infinite loop, no HALT
unused1: ldc 1
top:     adc 1
         brz top        ; conditional backward jump: normal loop, no warning
         br top         ; unconditional backward jump: warning
unused2: ldc 2
