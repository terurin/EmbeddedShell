#include "shell.h"
#include <stddef.h>

#include <string.h>
#include <stdio.h>

extern file_t current; //現在のパス
extern file_t root; //最上位パス
extern file_t lst_quick[]; //クイックリスト

static int bin_exit(int argc, char **argv) {
    printf("exit\n");
    exit(8);
}

static int bin_echo(int argc, char** argv) {
    if (argc > 1) {
        printf("%s\n", argv[1]);
    }
}

static int bin_list(int argc, char** argv) {
    file_t it;
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
    if (path == root)return;
    bin_pwd_sub(path->parent);
    printf("/%s", path->name);
}

static int bin_pwd(int argc, char** argv) {
    if (current != root) {
        bin_pwd_sub(current); //面倒くさいのでスタックを使う。
    } else {
        printf("/");
    }
    printf("\n");
    return 0;
}

static void bin_tree_sub(int depth, file_t path) {
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

static int bin_tree(int argc, char** argv) {
    bin_tree_sub(0, current);
    return 0;
}

static int bin_change_directory(int argc, char** argv) {
    file_t it;
    if (argc > 1) {
        it = path_get_all(argv[1]);
        if (it != NULL) {
            if ((it->fags & FileTypeMask) == FileTypeDirectory) {
                current = it;
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

static int bin_system(int argc, char** argv) {
    printf("memory:used[%d],release[%d]", gc_count_used(), gc_count_free());

}

static int bin_chmod(int argc, char** argv) {


}

static int bin_set(int argc, char** argv) {
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

static int bin_get(int argc, char** argv) {
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

static float temp = 100;

file_t shell_init() {
    file_t bin, env;
    gc_init();
    root = directory_create("root");
    directory_insert(root, bin = directory_create("bin"));
    directory_registor(bin);
    {
        directory_insert(bin, execute_create("ls", bin_list));
        directory_insert(bin, execute_create("tree", bin_tree));
        directory_insert(bin, execute_create("echo", bin_echo));
        directory_insert(bin, execute_create("pwd", bin_pwd));
        directory_insert(bin, execute_create("cd", bin_change_directory));
        directory_insert(bin, execute_create("set", bin_set));
        directory_insert(bin, execute_create("get", bin_get));
    }

    directory_insert(root, env = directory_create("env"));
    {
        directory_insert(root, float_create("temp", &temp, AccessReadAndWrite));
    }


    return current = root;
}

