//  ==============================================================================
//  Author       : Avni Agarwal
//  Roll No.     : 2501AI27
//  Declaration of Authorship:
//  I hereby declare that this code is my own original work.
//  ==============================================================================




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


// // parser.c - Simple extraction, no symbol table knowledge
// int parser_parse_line(const char *line, ParserResult *result) {
//     // Extract tokens, set result fields
//     // Return 1 = success, 0 = empty/ignored line
// }

// // Usage in pass1:
// ParserResult res;
// parser_parse_line(line, &res);
// if (res.has_label) symtab_insert(res.label, pc);
// if (res.has_mnemonic) { /* count instruction */ }

// void pass1(const char *filename) {
//     FILE *fp = fopen(filename, "r");
//     char line[256];
//     int address = 0;  // PC starts at 0
    
//     while (fgets(line, sizeof(line), fp)) {
//         char label[64], mnemonic[64], operand[64];
//         parser_parse_line(line, label, mnemonic, operand, NULL);
        
//         if (strlen(label) > 0) {
//             // Add to symbol table: label -> current PC
//             symtab_insert(label, address);
//             // Advance PC past this instruction
//         }

// // Usage in pass2:
// parser_parse_line(line, &res);
// int addr = symtab_lookup(res.operand); // Different use!
// pass2.c logic:
// void pass2(const char *filename, SymbolTable *symtab) {
//     FILE *fp = fopen(filename, "r");
//     char line[256];
//     int address = 0;
    
//     while (fgets(line, sizeof(line), fp)) {
//         char label[64], mnemonic[64], operand[64];
//         parser_parse_line(line, label, mnemonic, operand, NULL);
        
//         // Look up operand in symbol table if it's a label
//         int resolved_opand = 0;
//         if (is_label(operand)) {
//             resolved_opand = symtab_lookup(operand);  // get address
//         }
        
//         // Convert mnemonic to opcode
//         int opcode = opcodetable_lookup(mnemonic);
        
//         // Build 32-bit instruction: 8-bit opcode | 24-bit operand
//         uint32_t instruction = (opcode << 24) | (resolved_opand & 0xFFFFFF);
        
//         // Output to listing file
//         fprintf(listing_fp, "%04X %08X\n", address, instruction);
        
//         address++;
//     }
//     fclose(fp);
// }




// parser.c - uses fgets + strtok/sscanf:
// #include <stdio.h>
// #include <string.h>
// #include "parser.h"

// int parser_parse_line(const char *line, char *label, char *mnemonic, 
//                       char *operand, char *comment) {
//     char buffer[256];
//     strncpy(buffer, line, sizeof(buffer)-1);
//     buffer[sizeof(buffer)-1] = '\0';
    
//     // Remove comments first (everything after ';')
//     char *semicolon = strchr(buffer, ';');
//     if (semicolon) *semicolon = '\0';
    
//     // Trim leading whitespace
//     char *p = buffer;
//     while (*p == ' ' || *p == '\t') p++;
    
//     // Empty line check
//     if (*p == '\0' || *p == '\n') return 0;
    
//     // Extract label (if ends with ':')
//     *label = '\0'; *mnemonic = '\0'; *operand = '\0'; *comment = '\0';
    
//     // Check for label
//     if (p[strlen(p)-1] == ':') {
//         // found
//         *strtok(p, ":") = ...; // extract label name
//         p = strtok(NULL, " \t");
//     }
    
//     // Extract mnemonic
//     if (p) strncpy(mnemonic, p, ...);
//     p = strtok(NULL, " \t");
    
//     // Extract operand
//     if (p) strncpy(operand, p, ...);
    
//     return 1; // success
// }


// A. The Parser Function (parse_line)
// The parser's job is line-level syntax validation:   
// PDF
// + 1
// Invalid Label Name: Check if the label string is non-empty and starts with a letter (isalpha()) and contains only alphanumeric characters (isalnum()). If it starts with a number like 0def:, flag a Bogus Label Error.   
// PDF
// + 1
// Unknown Mnemonic: Look up mnemonic in your instruction table. If not found, flag an Unknown Instruction Error.   
// PDF
// + 3
// Operand Presence Check: Look up expected operand counts from your instruction lookup table:   
// PDF
// If the instruction requires an operand but operand is empty, flag Missing Operand Error.   
// PDF
// If the instruction requires no operand (e.g., add, return, HALT, mul, div) but operand is non-empty, flag Unexpected Operand Error.   
// PDF
// + 1
// Extra Garbage Check: Check for trailing tokens/junk past the expected operand (e.g., ldc 5, 6) and flag Extra Tokens Error.   
// PDF
// Number Formatting Check: If the operand is numeric, use strtol(). If strtol stops early (leaves trailing unparsed letters, like 08ge), flag an Invalid Number Error.





void parser(const char* line, ParserResult* result){
    //gets is tricky because it will scan until enter is there. so if variable is small, it'll overflow and corrupt the things stored after it. same with fscanf and if it expects a datatype and gets something else itll go in infinite loop. 
    //fgets +strtok/sscanf: fgets asks for array size beforehand so no overflow. sscanf to extract 
    char *line_cpy;
    // to find first occurence of ';'
    char *comment;
    int len_line;
    len_line= sizeof(*line);
    line_cpy= malloc(len_line+1);
    strncpy(line_cpy, line, len_line);
    // strtok(line_cpy, ";"); cant use strtok here because if ; is the first element, it will skip it. so using strchr to find first occurence and putting a \0 there
    comment= strchr(line_cpy,  ';');
    if(comment){
        *comment= '\0';
    }
    //now the line_cpy doesnt contain any comments
    
    result->label= strtok(line_cpy, ":");

}