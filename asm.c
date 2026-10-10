//  ==============================================================================
//  Author       : Avni Agarwal
//  Roll No.     : 2501AI27
//  Declaration of Authorship:
//  I hereby declare that this code is my own original work.
//  ==============================================================================




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char label[64];
    char mnemonic[64];
    char operand[64];
    int has_label;
    int has_mnemonic;
    int has_operand;
    int is_comment_only;
}ParserResult;

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