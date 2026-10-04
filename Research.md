What is Assembly?


ISO C89
    Standard C
Assembler
    Converts assembly to machine code.
    Translation
Emulator
    program that acts like fake hardware. basically, it will keep track of registers, memory etc.
    runs the binary code given by assembler by activating physical CPU and memory.
    execution
EMU program

Processor
    basically the instruction set they gave us (MIPS is also an example).
    pseudo assembly code with registers memory and program counter. 
Program Counter/ Instruction Pointer
    register keeping track of where the processor is in a program (bookmark).
    holds memory address of next instruction that processor will execute.


Project Specs:

    "Hardware"
        4 registers- 32 bits (8 bits- opcode, 24 bits for operand)
            2 are stack
            1 PC
            1 Stack pointer
        operand is 2's complement (with sign)
    Assembler:
        Line based
        comments ';'
        blank lines are permitted
        white space at start are allowed
        label names are followed by a ':'
        will produce a listing file (human readable)
            address followed by zero or one 32 bit value (in hex)
            ![alt text](image.png)
            any of these are acceptable
        should issue error warning
         


