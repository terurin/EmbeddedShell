#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main (){
    char buf[256];

    shell_init();

    while (1){
        gets(buf);
        shell_system(buf);
    }
    return 0;
}
