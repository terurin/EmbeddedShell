#include "shell_bin.h"
#include "shell_core.h"
#include "shell.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef int(*command_t)(int,char**);
typedef struct {
    const char* name;
    command_t command;
}file_pair;

const file_pair bins[]={
    {"exit",bin_exit},
    {"echo",bin_echo},
    {"ls",bin_ls},
    {"pwd",bin_pwd},
    {"tree",bin_tree},
    {"cd",bin_cd},
    {"system",bin_system},
    {"set",bin_set},
    {"get",bin_get}
};

void bin_registor(){
    file_t root=path_root();
    file_t bin;
    int i;
    directory_insert(root, bin = directory_create("bin"));
    directory_registor(bin);
    for (i=0;i<sizeof(bins)/sizeof(bins[0]);i++){
        directory_insert(bin,execute_create(bins[i].name,bins[i].command));
    }
}

int bin_exit(int argc, char **argv) {
    printf("exit\n");
    exit(8);
}

int bin_echo(int argc, char** argv) {
    if (argc > 1) {
        printf("%s\n", argv[1]);
    }
}

int bin_ls(int argc, char** argv) {
    file_t it;
    file_t current= path_current();
    if (argc == 1) {
        for (it = current->child; it != NULL; it = it->next) {
            printf("%s,", it->name);
        }
        printf("\n");
    } else if (argc == 2) {
        if (!strcmp(argv[1], "all")) {
            for (it = current->child; it != NULL; it = it->next) {
                printf("%s[%d]\n", it->name, it->fags & AccessMask);
            }
            printf("\n");
        }
    }
    return 0;
}

static void bin_pwd_sub(file_t path) {
    if (path == path_root())return;
    bin_pwd_sub(path->parent);
    printf("/%s", path->name);
}

int bin_pwd(int argc, char** argv) {
    file_t current= path_current();
    if (current != path_root()) {
        bin_pwd_sub(current); //面倒くさいのでスタックを使う。
    } else {
        printf("/");
    }
    printf("\n");
    return 0;
}

void bin_tree_sub(int depth, file_t path) {
    int i;
    file_t it;
    if (path == NULL)return;
    for (i = 0; i < depth; i++) {
        printf("|");
    }

    if (path->fags & FileTypeMask == FileTypeDirectory) {
        printf("%s*\n", path->name);
        for (it = path->child; it != NULL; it = it->next) {
            bin_tree_sub(depth + 1, it);
        }
    } else {
        printf("%s\n", path->name);
    }

}

int bin_tree(int argc, char** argv) {
    bin_tree_sub(0, path_current());
    return 0;
}

int bin_cd(int argc, char** argv) {
    file_t it;
    if (argc > 1) {
        it = path_get_all(argv[1]);
        if (it != NULL) {
            if ((it->fags & FileTypeMask) == FileTypeDirectory) {
                path_move(it);
            } else {
                puts("error:not directory");
            }
        } else {
            puts("found out");
            return -1;
        }
    }
    return 0;
}

int bin_system(int argc, char** argv) {
    printf("memory:used[%d],release[%d]", gc_count_used(), gc_count_free());

}

int bin_set(int argc, char** argv) {
    int cnt = 1;
    file_t file;
    int t;
    if (argc<=2){
        printf("Error:Few inputs\n");
        return -1;
    }
    
    while (cnt + 1 < argc) {
        file = path_get(argv[cnt]);
        if (file != NULL) {
            if (file->fags & AccessWrite) {
                switch (file->fags & FileTypeMask) {
                    case FileTypeDirectory:
                        printf("Error:Don't operate directory\n");
                        break;
                    case FileTypeNone:
                        printf("Error:FileTypeNone\n");
                        break;
                    case FileTypeInteger:
                        *file->ptr_int = atoi(argv[cnt + 1]);
                        break;
                    case FileTypeFloat:
                        *file->ptr_float = atof(argv[cnt + 1]);
                        break;
                    default:
                        printf("Error:Non Support\n");
                }
            } else {
                printf("Error:Access\n");
            }
        }else{
            printf("Error:Not Exist\n");
        }
        cnt += 2;
    }
    
    return 0;
}

int bin_get(int argc, char** argv) {
    int cnt = 1;
    file_t file;
    int temp;
    while (cnt < argc) {
        file = path_get(argv[cnt]);
        if (file != NULL) {
            if (file->fags & AccessWrite) {
                switch (file->fags & FileTypeMask) {
                    case FileTypeDirectory:
                        printf("Error:Don't operate directory\n");
                        break;
                    case FileTypeNone:
                        printf("Error:FileTypeNone\n");
                        break;
                    case FileTypeInteger:
                        printf("%d\n", *file->ptr_int);
                        break;
                    case FileTypeFloat:
                        printf("%f\n", *file->ptr_float);
                        break;
                    default:
                        printf("Error:Non Support\n");
                }
            } else {
                printf("Error:Access\n");
            }
            cnt += 1;
        }
    }
}
