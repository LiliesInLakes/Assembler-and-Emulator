/* ==============================================================================
 Author       : Avni Agarwal
 Roll No.     : 2501AI27
 Declaration of Authorship:
 I hereby declare that this code is my own original work.
 ==============================================================================
 Usage: emu [-trace] file.o

   -trace : show every instruction as it runs, along with A, B and SP.

 Once the program stops, whatever is sitting in memory gets written to the
 file "memory_dump.txt" (4 words per line, in hex). You don't have to ask
 for it, it always happens.

 Compile with:  gcc -std=c89 -pedantic -Wall emu.c -o emu
 ============================================================================== */

#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE      65536L      /* how many words of memory we have     */
#define MAX_INSTRUCTIONS 10000000L   /* give up after this many, probably an
                                        infinite loop                        */

/* Opcodes */
#define OP_LDC    0
#define OP_ADC    1
#define OP_LDL    2
#define OP_STL    3
#define OP_LDNL   4
#define OP_STNL   5
#define OP_ADD    6
#define OP_SUB    7
#define OP_SHL    8
#define OP_SHR    9
#define OP_ADJ    10
#define OP_A2SP   11
#define OP_SP2A   12
#define OP_CALL   13
#define OP_RETURN 14
#define OP_BRZ    15
#define OP_BRLZ   16
#define OP_BR     17
#define OP_HALT   18
#define OP_MUL    19
#define OP_DIV    20
#define TOTAL_OPCODES 21

/* Instruction names, only needed so -trace can print something readable */
const char *instruction_names[TOTAL_OPCODES] = {
    "ldc", "adc", "ldl", "stl", "ldnl", "stnl", "add", "sub", "shl", "shr",
    "adj", "a2sp", "sp2a", "call", "return", "brz", "brlz", "br", "HALT",
    "mul", "div"
};

/* 1 = this instruction takes an operand, 0 = it doesn't (again, just for -trace) */
int instruction_has_operand[TOTAL_OPCODES] = {
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 0, 0, 1, 0, 1, 1, 1, 0,
    0, 0
};

/* The machine itself. Every value is kept as a signed 32-bit number. */
long memory[MEMORY_SIZE];
long register_a = 0;
long register_b = 0;
long stack_pointer = 0;
long program_counter = 0;

long number_of_loaded_words = 0;
int trace_is_on = 0;


/* Turn a signed number into its unsigned 32-bit twin, so we can do maths on
   it without worrying about overflow. */
unsigned long to_unsigned_32(long value)
{
    return (unsigned long)value & 0xFFFFFFFFUL;
}


/* The opposite: take 32 bits and read them as a signed number. */
long to_signed_32(unsigned long value)
{
    value = value & 0xFFFFFFFFUL;
    if (value >= 0x80000000UL) {
        return -(long)(0xFFFFFFFFUL - value) - 1L;
    }
    return (long)value;
}


/* Work out base + offset, but only if it lands inside memory.
   Returns 1 and fills in *address if it is fine, 0 if it is out of range.
   The first check is there so the addition itself can never overflow. */
int find_address(long base, long offset, long *address)
{
    long result;

    if (base < -8388608L || base > MEMORY_SIZE + 8388608L) {
        return 0;
    }
    result = base + offset;
    if (result < 0 || result >= MEMORY_SIZE) {
        return 0;
    }
    *address = result;
    return 1;
}


/* Write the loaded part of memory into memory_dump.txt, 4 words a line,
   with the address of the first word at the start of each line. */
void write_memory_dump(void)
{
    FILE *dump_file;
    long word_number;

    dump_file = fopen("memory_dump.txt", "w");
    if (dump_file == NULL) {
        printf("Sorry, could not create memory_dump.txt\n");
        return;
    }

    for (word_number = 0; word_number < number_of_loaded_words; word_number++) {
        /* every 4th word we move to a new line and print the address */
        if (word_number % 4 == 0) {
            if (word_number != 0) {
                fprintf(dump_file, "\n");
            }
            fprintf(dump_file, "%08lX", (unsigned long)word_number);
        }
        fprintf(dump_file, " %08lX", to_unsigned_32(memory[word_number]));
    }
    fprintf(dump_file, "\n");

    fclose(dump_file);
    printf("Memory dump written to memory_dump.txt\n");
}


/* Show the registers on screen. */
void print_registers(void)
{
    printf("A=%ld  B=%ld  SP=%ld  PC=%ld\n",
           register_a, register_b, stack_pointer, program_counter);
}


/* Tell the user what went wrong and where. Always returns 1 so callers can
   just do "return show_runtime_error(...)". */
int show_runtime_error(const char *message, long error_pc)
{
    printf("RUNTIME ERROR at PC=%ld: %s\n", error_pc, message);
    print_registers();
    return 1;
}


/* Read the object file into memory. Returns 0 if all went well, 1 if not.
   Each word in the file is 4 bytes, least significant byte first. We build
   the number up byte by byte, which also means we don't care how big an int
   is on this computer. */
int load_object_file(const char *file_name)
{
    FILE *object_file;
    unsigned char bytes[4];
    unsigned long word_read;

    object_file = fopen(file_name, "rb");
    if (object_file == NULL) {
        printf("Cannot open object file '%s'\n", file_name);
        return 1;
    }

    while (fread(bytes, 1, 4, object_file) == 4) {
        if (number_of_loaded_words >= MEMORY_SIZE) {
            printf("Object file is too big for memory\n");
            fclose(object_file);
            return 1;
        }
        word_read = (unsigned long)bytes[0]
                  + (unsigned long)bytes[1] * 256UL
                  + (unsigned long)bytes[2] * 65536UL
                  + (unsigned long)bytes[3] * 16777216UL;
        memory[number_of_loaded_words] = to_signed_32(word_read);
        number_of_loaded_words++;
    }

    fclose(object_file);
    return 0;
}


/* Run the program. Returns 0 if it ended with HALT, 1 if something broke. */
int run_program(void)
{
    long instructions_executed = 0;
    unsigned long instruction_word;
    unsigned long operand_bits;
    unsigned long unsigned_result;
    long opcode;
    long operand;
    long address;
    long address_of_this_instruction;
    long shift_count;
    long shift_counter;
    long result;

    program_counter = 0;
    stack_pointer = MEMORY_SIZE;   /* stack starts at the very top and grows down */

    while (1) {
        if (program_counter < 0 || program_counter >= MEMORY_SIZE) {
            return show_runtime_error("PC has wandered outside memory",
                                      program_counter);
        }

        instructions_executed++;
        if (instructions_executed > MAX_INSTRUCTIONS) {
            return show_runtime_error("too many instructions (infinite loop?)",
                                      program_counter);
        }

        /* Step 1: fetch the instruction, then move PC on to the next one */
        address_of_this_instruction = program_counter;
        instruction_word = to_unsigned_32(memory[program_counter]);
        program_counter++;

        /* Step 2: decode it. The lowest 8 bits are the opcode and the top
           24 bits are the operand, which can be negative. */
        opcode = (long)(instruction_word % 256UL);
        operand_bits = instruction_word / 256UL;
        if (operand_bits >= 8388608UL) {
            operand = (long)operand_bits - 16777216L;   /* it was negative */
        } else {
            operand = (long)operand_bits;
        }

        if (opcode >= TOTAL_OPCODES) {
            return show_runtime_error("unknown opcode (are we running data?)",
                                      address_of_this_instruction);
        }

        if (trace_is_on) {
            printf("%08lX  %-6s", (unsigned long)address_of_this_instruction,
                   instruction_names[opcode]);
            if (instruction_has_operand[opcode]) {
                printf(" %-8ld", operand);
            } else {
                printf(" %-8s", "");
            }
            printf(" A=%ld B=%ld SP=%ld\n",
                   register_a, register_b, stack_pointer);
        }

        /* Step 3: actually do what the instruction says */
        if (opcode == OP_LDC) {
            register_b = register_a;
            register_a = operand;
        }
        else if (opcode == OP_ADC) {
            unsigned_result = to_unsigned_32(register_a) + to_unsigned_32(operand);
            register_a = to_signed_32(unsigned_result);
        }
        else if (opcode == OP_LDL) {
            if (!find_address(stack_pointer, operand, &address)) {
                return show_runtime_error("ldl address is outside memory",
                                          address_of_this_instruction);
            }
            register_b = register_a;
            register_a = memory[address];
        }
        else if (opcode == OP_STL) {
            if (!find_address(stack_pointer, operand, &address)) {
                return show_runtime_error("stl address is outside memory",
                                          address_of_this_instruction);
            }
            memory[address] = register_a;
            register_a = register_b;
        }
        else if (opcode == OP_LDNL) {
            if (!find_address(register_a, operand, &address)) {
                return show_runtime_error("ldnl address is outside memory",
                                          address_of_this_instruction);
            }
            register_a = memory[address];
        }
        else if (opcode == OP_STNL) {
            if (!find_address(register_a, operand, &address)) {
                return show_runtime_error("stnl address is outside memory",
                                          address_of_this_instruction);
            }
            memory[address] = register_b;
        }
        else if (opcode == OP_ADD) {
            unsigned_result = to_unsigned_32(register_b) + to_unsigned_32(register_a);
            register_a = to_signed_32(unsigned_result);
        }
        else if (opcode == OP_SUB) {
            unsigned_result = to_unsigned_32(register_b) - to_unsigned_32(register_a);
            register_a = to_signed_32(unsigned_result);
        }
        else if (opcode == OP_SHL) {
            shift_count = register_a;
            if (shift_count < 0 || shift_count > 31) {
                return show_runtime_error("shift count must be between 0 and 31",
                                          address_of_this_instruction);
            }
            /* shifting left is the same as doubling, shift_count times */
            unsigned_result = to_unsigned_32(register_b);
            for (shift_counter = 0; shift_counter < shift_count; shift_counter++) {
                unsigned_result = (unsigned_result * 2UL) & 0xFFFFFFFFUL;
            }
            register_a = to_signed_32(unsigned_result);
        }
        else if (opcode == OP_SHR) {
            shift_count = register_a;
            if (shift_count < 0 || shift_count > 31) {
                return show_runtime_error("shift count must be between 0 and 31",
                                          address_of_this_instruction);
            }
            /* shifting right is halving (rounding down), shift_count times.
               Negative numbers need a little care so they round down too. */
            result = register_b;
            for (shift_counter = 0; shift_counter < shift_count; shift_counter++) {
                if (result >= 0) {
                    result = result / 2;
                } else {
                    result = -((-(result + 1)) / 2) - 1;
                }
            }
            register_a = result;
        }
        else if (opcode == OP_ADJ) {
            stack_pointer = stack_pointer + operand;
            if (stack_pointer < 0 || stack_pointer > MEMORY_SIZE) {
                return show_runtime_error("SP has gone outside memory",
                                          address_of_this_instruction);
            }
        }
        else if (opcode == OP_A2SP) {
            stack_pointer = register_a;
            register_a = register_b;
            if (stack_pointer < 0 || stack_pointer > MEMORY_SIZE) {
                return show_runtime_error("SP has gone outside memory",
                                          address_of_this_instruction);
            }
        }
        else if (opcode == OP_SP2A) {
            register_b = register_a;
            register_a = stack_pointer;
        }
        else if (opcode == OP_CALL) {
            register_b = register_a;
            register_a = program_counter;      /* remember where to come back to */
            program_counter = program_counter + operand;
        }
        else if (opcode == OP_RETURN) {
            program_counter = register_a;
            register_a = register_b;
        }
        else if (opcode == OP_BRZ) {
            if (register_a == 0) {
                program_counter = program_counter + operand;
            }
        }
        else if (opcode == OP_BRLZ) {
            if (register_a < 0) {
                program_counter = program_counter + operand;
            }
        }
        else if (opcode == OP_BR) {
            program_counter = program_counter + operand;
        }
        else if (opcode == OP_HALT) {
            printf("HALT after %ld instructions\n", instructions_executed);
            return 0;
        }
        else if (opcode == OP_MUL) {
            /* unsigned maths simply wraps around on overflow, which is what
               we want here */
            unsigned_result = to_unsigned_32(register_b) * to_unsigned_32(register_a);
            register_a = to_signed_32(unsigned_result);
        }
        else if (opcode == OP_DIV) {
            if (register_a == 0) {
                return show_runtime_error("division by zero",
                                          address_of_this_instruction);
            }
            if (register_a == -1) {
                /* dividing by -1 is just negating. Doing it this way avoids
                   the one case (-2147483648 / -1) that would overflow. */
                unsigned_result = 0UL - to_unsigned_32(register_b);
                register_a = to_signed_32(unsigned_result);
            } else {
                register_a = register_b / register_a;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    int argument_number;
    int result_code;
    const char *object_file_name = NULL;

    /* Go through the command line. Anything that isn't -trace is taken to
       be the object file name. */
    for (argument_number = 1; argument_number < argc; argument_number++) {
        if (strcmp(argv[argument_number], "-trace") == 0) {
            trace_is_on = 1;
        } else {
            object_file_name = argv[argument_number];
        }
    }

    if (object_file_name == NULL) {
        printf("usage: emu [-trace] file.o\n");
        return 1;
    }

    if (load_object_file(object_file_name) != 0) {
        return 1;
    }
    printf("Loaded %ld words from %s\n", number_of_loaded_words, object_file_name);

    result_code = run_program();
    if (result_code == 0) {
        print_registers();
    }

    /* Always dump memory, even if the program crashed, since that is when
       you most want to look at it. */
    write_memory_dump();

    return result_code;
}
