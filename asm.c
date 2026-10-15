/* ==============================================================================
 Author       : Avni Agarwal
 Roll No.     : 2501AI27
 Declaration of Authorship:
 I hereby declare that this code is my own original work.
 ==============================================================================
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*Structure for instruction defination*/
typedef struct {
    const char *mnemonic;
    int opcode;
    int takes_operand;
} InstructionDef;

/*structure for symbol table*/
typedef struct {
    char name[64];
    int value;
} Symbol;

/*structure for parser function*/
typedef struct {
    char *label;
    char *mnemonic;
    char *operand;
    const char *error;
    int has_label;
    int has_mnemonic;
    int has_operand;
    int is_comment_only;

}ParserResult;



static const InstructionDef inst_table[] = {
    {"data", -1, 1}, 
    {"ldc",   0, 1},
    {"adc",   1, 1},
    {"ldl",   2, 1},
    {"stl",   3, 1},
    {"ldnl",  4, 1},
    {"stnl",  5, 1},
    {"add",   6, 0},
    {"sub",   7, 0},
    {"shl",   8, 0},
    {"shr",   9, 0},
    {"adj",  10, 1},
    {"a2sp", 11, 0},
    {"sp2a", 12, 0},
    {"call", 13, 1},
    {"return",14, 0},
    {"brz",  15, 1},
    {"brlz", 16, 1},
    {"br",   17, 1},
    {"HALT", 18, 0},
    {"mul",  19, 0},
    {"div",  20, 0},
    {NULL,   -1, 0}
};
int nerr=0;


int is_string_alphanumeric(char *str){
    int i;
    for(i=0; str[i]!='\0'; i++){
        if(!isalnum((unsigned char)str[i])){
            return 0;
        }
    }
    return 1;
}

char* trim_white_space(char *str){
    char* end;
    while(*str && isspace((unsigned char)*str)){
        str++;
    }
    if(*str!=0){
        end= str+ strlen(str)-1;
        while(end>str && isspace((unsigned char)*end)){
            end--;
        }
        *++end= '\0';
    }
    return str;
}


int parser(const char* line, ParserResult* result){

    char *line_cpy;
    char *comment;
    int len_line;
    char * colon_pos;
    char *token;
    char *head_of_line_cpy;
    colon_pos= NULL;

    if (!line || !result) return -1;

    result->label = NULL;
    result->mnemonic = NULL;
    result->operand = NULL;
    result->has_label=0;
    result->has_mnemonic = 0;
    result->has_operand = 0;
    result->is_comment_only = 0;
    result->error = NULL;
  


    /*gets is tricky because it will scan until enter is there. so if variable is small, it'll overflow and corrupt the things stored after it. same with fscanf and if it expects a datatype and gets something else itll go in infinite loop. 
    fgets +strtok/sscanf: fgets asks for array size beforehand so no overflow. sscanf to extract 
     to find first occurence of ';'*/
    len_line= strlen(line);
    line_cpy= malloc(len_line+1);
    head_of_line_cpy= line_cpy;
    strncpy(line_cpy, line, len_line);
    /* strtok(line_cpy, ";"); cant use strtok here because if ; is the first element, it will skip it. so using strchr to find first occurence and putting a \0 there*/
    comment= strchr(line_cpy,  ';');
    if(comment){
        *comment= '\0';
    }

    line_cpy= trim_white_space(line_cpy);
    if (*line_cpy == '\0') {
        result->is_comment_only = 1;
        free(head_of_line_cpy);
        return 0; /*comment only line*/
    }

    /*now the line_cpy doesnt contain any comments*/
    line_cpy= trim_white_space(line_cpy);
    colon_pos= strchr(line_cpy, ':');
    if(colon_pos){
        *colon_pos= '\0';
        if(isalpha((unsigned char)line_cpy[0]) && is_string_alphanumeric(line_cpy+1)){
            result->label= line_cpy;
            result->has_label=1;
        }
        else{
            result->error= "invalid label name (must be alphanumeric, start with a letter)";
            nerr++;
            free(head_of_line_cpy);
            return -1;
        }
        /*move past the colon*/
        line_cpy= trim_white_space(colon_pos+1);
        if(*line_cpy== '\0'){
            free(head_of_line_cpy);
            return 0; /*label only line*/
        }
    }

    /*extracting mnemonic*/
    result->mnemonic = strtok(line_cpy, " \t\r\n");
    if(result->mnemonic){
        result->has_mnemonic = 1;
        token = strtok(NULL, " \t\r\n;");
        if (token) {
            result->operand= trim_white_space(token);
            result->has_operand = 1;
        }
    }

    line_cpy= trim_white_space(line_cpy);
    if(*line_cpy != '\0'){
        printf("%s\n", line_cpy);
        result->error = "unexpected extra text after operand";
        nerr++;
        free(head_of_line_cpy);
        return -1;
    }
    free(head_of_line_cpy);
    return 0;
}

const InstructionDef *find_instruction(const char *mnemonic) {
    int i = 0;
    while (inst_table[i].mnemonic != NULL) {
        if (strcmp(inst_table[i].mnemonic, mnemonic) == 0) {
            return &inst_table[i];
        }
        i++;
    }
    return NULL;
}

void p1(FILE* fptr){
    char line_of_asm[64];
    ParserResult res;
    InstructionDef* pos_of_instruction;
    while(fgets(line_of_asm, sizeof(line_of_asm), fptr)){
        printf("going into parser");
        printf("line is %s\n", line_of_asm);
        parser(line_of_asm, &res);
        printf("%s %s %s \n%d %d %d", res.label, res.mnemonic, res.operand, res.has_label, res.has_mnemonic, res.is_comment_only);
        printf("%s\n", res.error);
        pos_of_instruction= find_instruction(res.mnemonic);
        if(!pos_of_instruction){
            res.error = "Unknown mnemonic";
            nerr++;
            continue;
        }
        /*also, opernad has comma i think. need to check and do we need to add erro if operands are non alphanumeric?*/
        /*if instructionr reqs operand and res has operand: put in symbol table
        if instruction reqs operand but res doesnt have operand: error
        if instruction doesnt req operand bu res has operand: error
        if instruction doesnt req operand and res doesnt have operand : put in symbol table*/
        if(pos_of_instruction->takes_operand ^ res.has_operand){
            res.error = "mnemonic's operand requirement is unfulfilled"; 
            nerr++;  
        }

        /*error checks, errors like???? more or less number of operands, not a number for operand. finding if instruction exists in the instruction table. */
    }
    /*will have to add error checks and also populate symbol table*/
}

int main(int argc, char **argv){
    FILE* fptr;
    
    if(argc == 2){
        fptr = fopen(argv[1], "r");
    }
    else{
        printf("invalid input to assembler\n");
        return 0;
    }
    if (fptr == NULL) {
        printf("The file is not opened.");
        return 0;
    }
    else{printf("file is opening\n");}
    p1(fptr);

    return 0;
}