//  ==============================================================================
//  Author       : Avni Agarwal
//  Roll No.     : 2501AI27
//  Declaration of Authorship:
//  I hereby declare that this code is my own original work.
//  ==============================================================================

#include <stdio.h>
#include <string.h>
#include <ctype.h>


int main(){
    char str[]= " a label on its own";
    char *test;
    test= strtok(str, ";");
    printf("%s", test);
    printf("-----");
    char *test2;
    test2= strtok(NULL, " ");
    printf("\n%s", test2);
    char * test3;
    test3= strchr(str,  ';');
    if(test3){
        *test3= '\0';
    }
    printf("\n----");
    printf("\n%s----", test3);
    char lab[10]; // Properly declare an array of characters
    
    lab[0] = '\0';
    if(isalpha((unsigned char)lab[0])){
        printf("yes");
    }
    else{printf("no");}
}
