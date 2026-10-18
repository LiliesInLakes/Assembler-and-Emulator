/* ==============================================================================
 Author       : Avni Agarwal
 Roll No.     : 2501AI27
 Declaration of Authorship:
 I hereby declare that this code is my own original work.
 ==============================================================================
*/


/*switch debug, 0 hai toh printing nahi hogi. also, strdub karlo toh theek ho jaayega*/
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
    int value; /*pc*/
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


/*in last column, 1 = takes operand, 0 means no operand*/
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
    {"SET", -1, 1},
    {NULL,   -1, 0}
};

static Symbol *symbol_table;
static int nsyms=0;
static int nerr = 0;
FILE *log_file = NULL;
int debug=0;


int find_symbol(char *label){
    int i;

    if(label == NULL){
        return -1;
    }

    for(i = 0 ; i < nsyms ; i++){
        if(debug){
            printf("find_symbol: symbol_table[i].name: %s, label: %s\n", symbol_table[i].name, label);
        }
        if(strcmp(symbol_table[i].name, label) == 0){
            if(debug){
                printf("find_symbol: returning 1\n");
            }
            return 1;
        }
    }
    if(debug){
            printf("find symbol: return 0\n");
    }
    return 0;
}


int add_to_symbol_table(ParserResult res, int pc){
    Symbol temp;
    Symbol *tmp;
    if(res.label == NULL){
        return -1;
    }
    if(find_symbol(res.label) == 1){
        if(debug){
            printf("found label in symbol table so returning 0\n");
        }
        return 0;
    }
    tmp = realloc(symbol_table, sizeof(Symbol)*(nsyms+1));
    if(!tmp){
        return -1;
    }
    symbol_table= tmp;
    if(debug){
        printf("add_to_symbol_table: realloc done\n");
    }
    strcpy(temp.name, res.label);
    temp.value= pc;
    if(debug){
        printf("add_to_symbol_table: temp created\n");
    }
    symbol_table[nsyms] = temp;
    if(debug){
        printf("added to symbol table\n");
    }
    nsyms++;
    if(debug){
            printf("returning 1 in add_to_symbol_table\n");
    }
    return 1;
}


const InstructionDef *find_instruction(const char *mnemonic) {
    int i = 0;
    if(mnemonic == NULL){
        return NULL;
    }
    while (inst_table[i].mnemonic != NULL) {
        if (strcmp(inst_table[i].mnemonic, mnemonic) == 0) {
            return &inst_table[i];
        }
        i++;
    }
    return NULL;
}


void add_error(const char* line, const char* error, ParserResult res){
    if(log_file){
        fprintf(log_file, "%s       | ERR: %s\n", line, error);
        fprintf(log_file, "     %s %s %s\n       %d %d %d %d\n", res.label, res.mnemonic, res.operand, res.has_label, res.has_mnemonic, res.has_operand, res.is_comment_only);
    }
}


int is_string_alphanumeric(char *str){
    int i;
    for(i = 0 ; str[i] != '\0' ; i++){
        if(!isalnum((unsigned char)str[i])){
            return 0;
        }
    }
    return 1;
}


char* trim_white_space(char* str){
    char* end;
    while(*str && isspace((unsigned char)* str)){
        str++;
    }
    if(*str != 0){
        end = str + strlen(str) - 1;
        while(end > str && isspace((unsigned char)* end)){
            end--;
        }
        *++end = '\0';
    }
    return str;
}


int parser(const char* line, ParserResult* result){

    char* line_cpy;
    char* comment;
    int len_line;
    char* colon_pos;
    char* head_of_linecpy;
    char* token;
    colon_pos = NULL;

    if (!line || !result) return -1;

    result->label = NULL;
    result->mnemonic = NULL;
    result->operand = NULL;
    result->has_label = 0;
    result->has_mnemonic = 0;
    result->has_operand = 0;
    result->is_comment_only = 0;
    result->error = NULL;
  


    /*gets is tricky because it will scan until enter is there. so if variable is small, it'll overflow and corrupt the things stored after it. same with fscanf and if it expects a datatype and gets something else itll go in infinite loop. 
    fgets +strtok/sscanf: fgets asks for array size beforehand so no overflow. sscanf to extract 
     to find first occurence of ';'*/
    len_line = strlen(line);
    line_cpy = malloc(len_line+1);
    head_of_linecpy= line_cpy;
    if(line_cpy == NULL){
        printf("memory not assigned to line_cpy in parser\n");
        free(head_of_linecpy);
        return 0;
    }
    strncpy(line_cpy, line, len_line);
    line_cpy[len_line] = '\0';
    if(debug){
        printf("parser: line_cpy: %s\n", line_cpy);
    }
    /* strtok(line_cpy, ";"); cant use strtok here because if ; is the first element, it will skip it. so using strchr to find first occurence and putting a \0 there*/
    comment = strchr(line_cpy,  ';');
    if(comment){
        *comment = '\0';
    }

    line_cpy = trim_white_space(line_cpy);
    if (*line_cpy == '\0') {
        result->is_comment_only = 1;
        free(head_of_linecpy);
        return 1; /*comment only line*/
    }

    /*now the line_cpy doesnt contain any comments*/
    line_cpy = trim_white_space(line_cpy);
    colon_pos = strchr(line_cpy, ':');
    if(colon_pos){
        *colon_pos = '\0';
        if(isalpha((unsigned char)line_cpy[0]) && is_string_alphanumeric(line_cpy+1)){
            result->label = strdup(line_cpy);
            result->has_label = 1;
        }
        else{
            result->error = "invalid label name (must be alphanumeric, start with a letter)";
            add_error(line, result->error, *result);
            nerr++;
            free(head_of_linecpy);
            return 0;
        }
        /*move past the colon*/
        line_cpy = trim_white_space(colon_pos+1);
        if(*line_cpy == '\0'){
            free(head_of_linecpy);
            return 1; /*label only line*/
        }
    }

    /*extracting mnemonic*/
    token = strtok(line_cpy, " \t\r\n");
    if(token){
        result->mnemonic= strdup(token);
        result->has_mnemonic = 1;
        token = strtok(NULL, " \t\r\n;");
        if (token) {
            result->operand = strdup(trim_white_space(token));
            result->has_operand = 1;
        }
        /*checking if text exists after operand*/
        token = strtok(NULL, " \t\r\n;");
        if(token){
            if(debug){printf("adding unexpected extra text after operand. the text is %s\n", line_cpy);}
            result->error = "unexpected extra text after operand";
            add_error(line, result->error, *result);
            nerr++;
            free(head_of_linecpy);
            return 1; /*because we still want to populate the symbol table with the mnemonic*/
        }
    }
    free(head_of_linecpy);
    return 1;
}


void print_symbol_table(FILE *out){
    int i;
    if(out == NULL){
        return;
    }
    fprintf(out, "Symbol table (%d entries)\n", nsyms);
    fprintf(out, "%-20s %s\n", "LABEL", "ADDRESS");
    for(i = 0; i < nsyms; i++){
        fprintf(out, "%-20s %08X\n", symbol_table[i].name, symbol_table[i].value);
    }
}

void pass1(FILE* fptr){
    int pc = 0;
    char line_of_asm[64]; /*then what is the point of keeping parserResult as dynamic if line is static ;(*/
    ParserResult res;
    int return_of_parser;
    const InstructionDef* pos_of_instruction;

    while(fgets(line_of_asm, sizeof(line_of_asm), fptr)){
        if(debug){
            printf("pass1: line: %s\n", line_of_asm);
        }
        return_of_parser = parser(line_of_asm, &res);
        if(!return_of_parser || res.is_comment_only){
            if(debug){
                printf("pass1: !return_of_parser || res.is_comment_only true\n");
            }
            continue; /*if parser result is 0 then there is some error in the line so we dont want to operate on it further. if it is all comments then also same*/
        }
        if(res.mnemonic){
            if(debug){
                printf("pass1: res.mnemonic true\n");
            }
            pos_of_instruction = find_instruction(res.mnemonic);
            if(!pos_of_instruction){
                res.error = "Unknown mnemonic";
                add_error(line_of_asm, res.error, res);
                nerr++;
                continue;
            }
            if(pos_of_instruction->takes_operand ^ res.has_operand){
                res.error = "mnemonic's operand requirement is unfulfilled"; 
                add_error(line_of_asm, res.error, res);
                nerr++;
                /*not doing continue here because we can still populate the symbol table and see if there are more errors. If */
            }
        }
        if(debug){
            printf("going to populate symbol table now\n");
        }
        if(res.has_label && !add_to_symbol_table(res, pc)){
            if(debug){
                printf("add to symbol table gave 0\n");
            }
            add_error(line_of_asm, "duplicate label", res);
            nerr++;
        }
        
        if(res.has_mnemonic){
            pc++;
        }
    }
    print_symbol_table(stdout);
    return;
}


void pass2(FILE* fptr){
    int pc = 0;
    long int operand_num;
    char* endptr = NULL;
    char line_of_asm[64];
    ParserResult res;
    int return_of_parser;
    const InstructionDef* pos_of_instruction;


    while(fgets(line_of_asm, sizeof(line_of_asm), fptr)){
        if(debug){
            printf("pass2: line: %s\n", line_of_asm);
        }
        return_of_parser = parser(line_of_asm, &res);
        if(!return_of_parser || res.is_comment_only || !res.has_mnemonic){
            if(debug){
                printf("pass2: !return_of_parser || res.is_comment_only true\n");
            }
            continue; /*if parser result is 0 then there is some error in the line so we dont want to operate on it further. if it is all comments then also same*/
        }
        if(res.mnemonic){
            if(debug){
                printf("pass2: res.mnemonic true\n");
            }
            pos_of_instruction = find_instruction(res.mnemonic);
            if(pos_of_instruction->takes_operand && res.has_operand){
                /*convert operand to number else treat as label and find in symbol table.*/
            }
            /* if this if is not satisfied then it will have already raised an error in pass1. 
            but then what about opcode??? 
            find opcode
            Write the listing (address, machine word in hex, source line) as you go, and write the binary object file with fwrite only if nerr == 0 at the end.
            */
            pc++;
        }
    }
    return;
}

int main(int argc, char **argv){
    FILE* fptr;
    log_file = fopen("assemble.log", "w");
    if(log_file == NULL){
        printf("The log file didnt open\n");
        return 1;
    }
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
    pass1(fptr);
    fclose(log_file);
    fclose(fptr);
    return 0;
}