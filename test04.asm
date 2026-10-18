; Author       : Avni Agarwal
; Roll No.     : 2501AI27
; Test 4: the line formats allowed by the spec. Checks that every legal way of laying out a line is accepted.
; just a comment

label1:                 ; label on its own
label2: ldc 5           ; label and instruction
        adc 5
label3:ldc label3       ; no space after colon
	ldc	label1  ; tabs as whitespace
here:   set 100         ; set pseudo-instruction (no code)
        ldc here        ; uses value 100
        brz fwd         ; forward branch
        ldc label2
fwd:    data fwd        ; data holding a label value
        halt            ; lowercase mnemonic
