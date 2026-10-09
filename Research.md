//  ==============================================================================
//  Author       : Avni Agarwal
//  Roll No.     : 2501AI27
//  ==============================================================================



What is Assembly?
    Assembly language uses a mnemonic to represent each low-level machine operation or opcod

ISO C89
    Standard C
Hash Function
    takes strings and other things to get an index according to some formula
Error
    prevents code generation and assembly
Warnings
    flags sus code but allows completion
IN THIS CODE THE ERRORS AND WARNINGS ARE:
    label name not following the naming rules- starting with digit or non alphanumeric stuff
    mnemonic not in instruction table
    instruction requiring operand has none
    instruction requiring no operands has one
    Trailing arguments or junk at the end of a line
    Operand is not a valid decimal, octal, hex, or label name
    non alhpanumeric elements in non comment portion


Assembler
    Converts assembly to machine code.
    Translation
    self assembler: runs on same computer it produces machine language code for
    cross assembler: runs of diff computer than the one it produces machine language for
    Single pass: generates opcode immediately in memory . but issue is it wants all the labels to be pre defined. If some new label comes, it is not able to resolve (soln: 2 pass).
    3 main data structures:
    opcode table
    location counter
    symbol table


**2 pass assembler**: if it encounters a label it doesnt recognise, it puts it in the symbol table along with the address where the undefined symbol's value has to be placed if encountered again. 
    First pass: 
        for forward references and generate new opcodes and doing recursion (check ref 2) Symbol table is generated. 
        The symbol table doesnt contain the mnemonics. only labels and their pc address
    Second pass: 
        symbolic opcode to machine understandable opcode. Program is read again and symbol table is used to get corresponding addresses associated with each label. Opcode table is is used to convert each instruction (mnemonic) to 32 bit machine code. (binary)
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

Opcode
    numerical code that tells processor which action to perform
Operand
    data or target value that the instruction acts on. this is not register (add, sub dont require operand. ldc 5 br loop have operand (target value or address))
    can be number or label name
Label
    Human readable bookmark given to specific memory address. Location in memory ko doosra naam de diya basically.
    since assembly doesnt have variables, it uses this to give symbolic names to memory addresses.
    $t0 is not a label though
    when a label comes, the pc (index for instruction) points to the address that the label points to (basically the index of the instruction that is labelled by the label)
Mnemonic
    action or command that processor has to do. human readable (ldc, add, br, etc) opcode


Project Specs:

    "Hardware"
        4 registers- 32 bits (8 bits- opcode, 24 bits for operand)
            2 are stack
            1 PC
            1 Stack pointer
        operand is 2's complement (with sign)
    Assembler:
        Single pass but two pass (check notes at page 5 of ps)
        Line based
        comments ';'
        blank lines are permitted
        white space at start are allowed
        label names are followed by a ':'
        alphanumeric label names are valid but has to start with alphabet
        operand is either label or number(hex, dec, oct)
        will produce a listing file (human readable)
            address followed by zero or one 32 bit value (in hex)
            ![alt text](image.png)
            any of these are acceptable
        should issue error warning
To explore:
    Hash table to store symbol tables.

What we have to do:
    Assembler
    Bubble Sort in IITPSx assembly
    Emulator
    Test Cases have to pass

References:
https://www.geeksforgeeks.org/compiler-design/introduction-of-assembler/
https://kshitij-taley19.medium.com/two-pass-assembler-3c193c612cb8
https://github.com/ayushoriginal/Two-pass-Assembler/blob/master/README.md
https://www.youtube.com/watch?v=BUZycVrWPa4

STEPS!!
    For parser:
        first get the lines
        add pc
        remove comments
        and split lines on : and update the pc as well
        give error warnings



qs??
    do we need to build a loader also?
    what about pseudo operations?
    what is stored in symbol table?
    will the literal and symbol table be different?



