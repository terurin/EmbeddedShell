/* 
 * File:   shell.h
 * Author: TERU
 *
 * Created on 2016/08/26, 15:37
 */

#ifndef SHELL_H
#define	SHELL_H
#include <stdint.h>
#include <stdbool.h>
//Shell用内部GC

enum PathType{
        PATH_ACCESS_MASK=0x0F,//アクセス権について
        PATH_TYPE_MASK=0xF0,//ファイルの種類について
        PATH_ACCESS_EXCUDE=0x01,
        PATH_ACCESS_WRITE=0x02,
        PATH_ACCESS_READ=0x04,
        PATH_ACCESS_LOCK=0x08,
        PATH_FILE_MASK=0xF0,
        PATH_DIRECTORY=0x10,
        PATH_EXECUTE_FUNCTION=0x21,//実行可能
        PATH_INTEGER=0x30,
        PATH_FLOAT=0x40
};

enum Access{
    ACCESS_NONE=0x00,
    ACCESS_EXCUDE=0x01,
    ACCESS_WRITE=0x02,
    ACCESS_READ=0x04,
    ACCESS_WRITE_READ=0x06,
    ACCESS_LOCK=0x08,
    ACCESS_MASK=0x0F
};
typedef enum PathType PathType_t;
typedef enum Access Access_t;

struct GC_Item {
    const char *name;
    const char *comment;
    struct GC_Item *parent, *next; //木構造を作成
    PathType_t type;//unionの使い方を決定する。
    union{//
        int (*func)(int, char**);
        struct GC_Item *child;//子ディレクトリ
        int* num32;
    };
    unsigned int mode;
};

union GCBody {
    struct GC_Item item; //この要素は位置を変えてはいけない。
    union GCBody *next; //単方向リストを構成する。

};
typedef union GCBody GC_Body_t;

void gc_init();
unsigned int gc_count_free(); //O(n)
unsigned int gc_count_used(); //O(n)

typedef struct GC_Item* path_t;
//これらは単純にメモリーを確保、開放しているだけである。よってこれらは通常呼び出さない。
path_t gc_path_create(const char *name);
void gc_path_destroy(path_t);
//やっと本格的な操作
int path_insert(path_t brother,path_t it);//同位に要素を追加(root用)(親が追加されないので注意)

int path_tags(path_t);



path_t path_get(const char*);//名前からpathを取得する。 

path_t directory_create(const char *name);
int directory_insert(path_t directory,path_t file);
int directory_registor(path_t);//Pathに登録する。
void directory_registor_init();

path_t execute_create(const char* name,int(*)(int,char**));//そのまま実行可能なものつまり関数。


path_t integer_create(const char* name,int* ptr,Access_t acc);

int shell_system(char* name);
int shell_system_s(int argc,char** argv);

int shell_error();

path_t shell_init();

#endif	/* SHELL_H */

