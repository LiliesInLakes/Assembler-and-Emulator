#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char label[64];
    char mnemonic[64];
    char operand[64];
    int has_label;
    int has_mnemonic;
    int has_operand;
    int is_comment_only;
}ParserResult;
//not giving direct edit access of symbol table to parser because then when we want to reuse in pass 2, it wont be possible. Pass 2 only wants lookup not edit. 
// if we give file access, the file pointer will be moved and then there would be tangling in the pass 2.  
void parser_func(const char* line, ParserResult *result);

#endif