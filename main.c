#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
int exit_func(int argc,char **argv){
    exit(0);
}

int echo_func(int argc,char ** argv){
    puts(argv[0]);
    if (argc>1){
        puts(argv[1]);
    }
} 


int main (){
    char buf[256];

    shell_init();

    while (1){
        gets(buf);
        shell_system(buf);
    }
    return 0;
}
