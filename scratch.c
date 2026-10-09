#include <stdio.h>
#include <string.h>


int main(){
    char str[]= "; a label on its own";
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
}
