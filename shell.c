#include "shell.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#define GC_SIZE 64
#define SPLIT_MAX 32
#define PATH_DEPTH_MAX 8
#define QUICK_MAX 8 
#define WORD_MAX 32
static GC_Body_t lst_gc[GC_SIZE], *gc_free; //GCの管理領域

static int error; //エラーの有無

static const char split[] = " \r\n";
static path_t current; //現在のパス
static path_t root; //最上位パス
static path_t lst_quick[QUICK_MAX]; //クイックリスト

void gc_init() {
    uint16_t i;
    path_t p;
    //make single link sturcture
    gc_free = &lst_gc[0];
    for (i = 0; i < GC_SIZE - 1; i++) {
        lst_gc[i].next = &lst_gc[i + 1];
    }
    lst_gc[GC_SIZE - 1].next = NULL;
    //stop bag
    for (i = 0; i < GC_SIZE - 1; i++) {
        p = (path_t) & lst_gc[i];
        p->next = NULL;
        p->parent = NULL;
        p->child = NULL;
    }

}

unsigned int gc_count_free() {
    GC_Body_t *it;
    unsigned int cnt = 1;
    if (gc_free == NULL)return 0;
    for (it = gc_free; it->next != NULL; it = it->next) {
        cnt++;
    }
    return cnt;
}

unsigned int gc_count_used() {
    return GC_SIZE - gc_count_free();
}

path_t gc_path_create(const char *name) {
    GC_Body_t *mem;
    path_t path;
    //メモリー配置(pop)
    if (gc_free == NULL)return NULL; //メモリーが配置できない。
    mem = (GC_Body_t*) gc_free;
    gc_free = gc_free->next;
    mem->next = NULL;
    path = (path_t) mem;
    //木構造を初期化
    path->parent = NULL;
    path->child = NULL; //一応NULLにしておく
    path->next = NULL;
    path->name = name;
    return path;
}

void gc_path_destroy(path_t path) {
    //push動作
    GC_Body_t* mem = (GC_Body_t*) path;
    mem->next = gc_free;
    gc_free = mem;
}

int path_insert(path_t brother, path_t it) {
    if (brother == NULL)return -1;
    if (it == NULL)return -2;
    //brother
    it->next = brother->next;
    brother->next = it;
}

int path_tags(path_t path) {
    if (path == NULL)return 0;
}

static path_t path_get_main(const char* name) {
    const char split[] = "/"; //Directory用区切り文字
    path_t it;
    bool fag = true;
    char buf[WORD_MAX], *p, *word;
    if (name == NULL)return NULL;
    strcpy(buf, name); //const外しを行う
    p = buf;
    if (*p == '/') {
        it = root;
        p++;
        if (*p == 0)return root;
    } else {
        it = current;
    }

    it = it->child;
    word = strtok(p, split);
    while (word != NULL) {
        while (!strcmp(".", word)) {
            word = strtok(NULL, split);
            if (word == NULL)return it;
        }

        while (!strcmp("..", word)) {
            //初回は下がりすぎているので2つ戻す。
            if (fag) {
                it = it->parent;
                fag = false;
            }

            it = it->parent;
            word = strtok(NULL, split);
            if (word == NULL)return it;
        }

        if (!strcmp(word, it->name)) {
            word = strtok(NULL, split);
            if (word == NULL) {
                return it;
            } else {
                it = it->child;
            }
        } else {
            it = it->next;
            if (it == NULL)return NULL;
        }

    }
    return it;
}

path_t path_get_registor(const char* name) {
    int idx;
    path_t it;
    if (name == NULL)return NULL;
    for (idx = 0; idx < QUICK_MAX; idx++) {
        if (lst_quick[idx] != NULL) {
            for (it = lst_quick[idx]->child; it != NULL; it = it->next) {
                if (!strcmp(name, it->name)) {
                    return it;
                }
            }
        }
    }
    return NULL;
}

path_t path_get(const char* name) {
    path_t it;
    int idx;
    it = path_get_main(name);
    if (it != NULL)return it;
    it = path_get_registor(name);
    if (it != NULL)return it;
    return NULL;
}

path_t directory_create(const char *name) {
    path_t path = gc_path_create(name);

    if (name == NULL)return NULL;
    path->type = PATH_DIRECTORY;
    path->child = NULL; //まだ子要素は存在しない。
    return path;
}

int directory_insert(path_t directory, path_t file) {
    path_t it;
    if (directory == NULL)return -1; //error
    if (file == NULL)return -2;
    if (directory->type != PATH_DIRECTORY)return -3; //error

    //メモリー配置を行う。
    file->parent = directory;
    file->next = directory->child;
    directory->child = file;

    return 0;
}

int directory_registor(path_t dir) {
    if (dir == NULL)return -1;
    int idx;
    for (idx = 0; idx < QUICK_MAX; idx++) {
        if (lst_quick[idx] == NULL) {
            lst_quick[idx] = dir;
            return 0;
        }
    }
    return -1;
}

void directory_registor_init() {
    int idx;
    for (idx = 0; idx < QUICK_MAX; idx++) {
        lst_quick[idx] = NULL;
    }
}

path_t execute_create(const char* name, int(*func)(int, char**)) {
    path_t path = gc_path_create(name);
    if (path == NULL)return NULL;
    path->type = PATH_EXECUTE_FUNCTION;
    path->func = func;
    return path;
}

path_t integer_create(const char* name, int* ptr, Access_t acc) {
    path_t path = gc_path_create(name);
    if (path == NULL)return NULL;
    if (ptr == NULL)return NULL;
    path->type = (acc & ACCESS_MASK) | PATH_INTEGER;

    return path;
}

int shell_system(char *command) {
    int argc = 0;
    char *argv[SPLIT_MAX], *temp;

    argv[argc] = temp = strtok(command, split);
    while (temp != NULL && argc < SPLIT_MAX) {
        argv[++argc] = temp = strtok(NULL, split);
    }
    return shell_system_s(argc, argv);
}

int shell_system_s(int argc, char* argv[]) {
    path_t it;
    it = path_get(argv[0]);
    if (it == NULL)return -1;
    switch (it->type) {
        case PATH_EXECUTE_FUNCTION:
            return it->func(argc, argv);
        default:
            puts("error");
            return -3;
    }
}

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
    path_t it;
    if (argc == 1) {
        for (it = current->child; it != NULL; it = it->next) {
            printf("%s,", it->name);
        }
        printf("\n");
    } else if (argc == 2) {
        if (!strcmp(argv[1], "all")) {
            for (it = current->child; it != NULL; it = it->next) {
                printf("%s[%d]\n", it->name, it->type & PATH_ACCESS_MASK);
            }
            printf("\n");
        }
    }


    return 0;
}

static void bin_pwd_sub(path_t path) {
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

static void bin_tree_sub(int depth, path_t path) {
    int i;
    path_t it;
    if (path == NULL)return;
    for (i = 0; i < depth; i++) {
        printf("|");
    }

    if (path->type == PATH_DIRECTORY) {
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
    path_t it;
    if (argc > 1) {
        it = path_get_main(argv[1]);
        if (it != NULL) {
            current = it;
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
    path_t file;
    PathType_t type;

    while (cnt < argc) {
        file = path_get_main(argv[cnt]);
        if (file != NULL) {

            cnt += 2;
        }
    }

}

path_t shell_init() {
    path_t bin, env;
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

    }
    env = directory_create("env");


    return current = root;
}

