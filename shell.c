#include "shell.h"
#include "shell_bin.h"
#include <stddef.h>

#include <string.h>
#include <stdio.h>

static float temp = 100;

void shell_init() {
    file_t root,env;
    path_init();//path構成を設定。
    bin_registor();//binを設定する。
    root=path_root();
    
    directory_insert(root, env = directory_create("env"));
    {
        directory_insert(root, float_create("temp", &temp, AccessReadAndWrite));
    }
}

