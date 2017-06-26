int main() { 
    extern char** environ;
    char** env = environ;
    while(*env != NULL) {
        printf("%s\n", *env++);
    }
    char* value = NULL; 
    env = environ;
    while(*env != NULL){
        char* str = *env++;
        if(strncmp(str, "LANG", strlen("LANG")) == 0) {
            value = str + strlen("LANG") + 1;
            break;
        }
    }
    printf("LANG=%s\n", value);  
}

#include <stdlib.h>

typedef int (*fun)(const char *command);

int main(void) {
    fun p = system;

    p("ls -al");

    return 0;
}