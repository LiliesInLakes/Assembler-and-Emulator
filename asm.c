//  ==============================================================================
//  Author       : Avni Agarwal
//  Roll No.     : 2501AI27
//  Declaration of Authorship:
//  I hereby declare that this code is my own original work.
//  ==============================================================================




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *label;
    char *mnemonic;
    char *operand;
    // int has_label;
    int has_mnemonic;
    int has_operand;
    int is_comment_only;
}ParserResult;


char* trim_white_space(char *str){
    char* end;
    while(isspace((unsigned char)*str)){
        str++;
    }
    if(*str!=0){
        end= str+ strlen(str)-1;
        while(isspace((unsigned char)*end)){
            end--;
        }
        *++end= '\0';
    }
    return str;
}


int parser(const char* line, ParserResult* result){

    if (!line || !result) return -1;

    result->has_mnemonic = 0;
    result->has_operand = 0;
    result->is_comment_only = 0;
    result->label[0] = '\0';
    result->mnemonic[0] = '\0';
    result->operand[0] = '\0';
    result->is_comment_only = 0;



    //gets is tricky because it will scan until enter is there. so if variable is small, it'll overflow and corrupt the things stored after it. same with fscanf and if it expects a datatype and gets something else itll go in infinite loop. 
    //fgets +strtok/sscanf: fgets asks for array size beforehand so no overflow. sscanf to extract 
    char *line_cpy;
    // to find first occurence of ';'
    char *comment;
    int len_line;
    len_line= strlen(line);
    line_cpy= malloc(len_line+1);
    strncpy(line_cpy, line, len_line);
    // strtok(line_cpy, ";"); cant use strtok here because if ; is the first element, it will skip it. so using strchr to find first occurence and putting a \0 there
    comment= strchr(line_cpy,  ';');
    if(comment){
        *comment= '\0';
    }
    //now the line_cpy doesnt contain any comments
    
    char * colon_pos= strchr(line_cpy, ':');
    if(colon_pos){
        *colon_pos= '\0';
    }
    char *lab= trim_white_space(line_cpy);
    if(isalpha((unsigned char)*lab)){
        strcpy(result->label, lab);
    }
    
    //need to check errors in label (empty label or non alphanumeric or illegal naming)

    line_cpy= colon_pos+1; //move past the colon
    line_cpy= trim_white_space(line_cpy);

    free(line_cpy);
}

int main(){
    ParserResult res;
    const char *test_line = "LOOP: MOV AX, BX ; this is a comment";

    parser(test_line, &res);

    if (res.label) {
        printf("Label found: %s\n", res.label);
    } else {
        printf("No label found.\n");
    }

    return 0;
}