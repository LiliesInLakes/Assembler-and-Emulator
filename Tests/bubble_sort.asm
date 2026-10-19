;  Author       : Avni Agarwal
;  Roll No.     : 2501AI27

size:   data 5              ; Number of elements in the array
array:  data 4              ; array[0] = 4
        data 2              ; array[1] = 2
        data 7              ; array[2] = 7
        data 1              ; array[3] = 1
        data 9              ; array[4] = 9


start:
        ldc size            ; Load address of 'size'
        ldnl 0              ; A = n (array length)
        
        ; Outer loop counter (i) initialized to 0
        ldc 0               ;
        
outerloop:
        ; Check if outer loop is finished (i == n - 1)
        ; For simplicity, we can track remaining passes or use a countdown.
        ; Let's implement a countdown for outer loop: i from n-1 down to 1.
        
        HALT               