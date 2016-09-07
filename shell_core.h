/* 
 * File:   shell_core.h
 * Author: TERU
 *
 * Created on 2016/09/07, 16:48
 */

#ifndef SHELL_CORE_H
#define	SHELL_CORE_H
#include "gcmem.h"

int path_insert(file_t brother, file_t it); //同位に要素を追加(root用)(親が追加されないので注意)
//ファイルの基本属性について
Access_t path_access_get(file_t);
void path_access_set(file_t,Access_t);
FileType_t path_filetype_get(file_t);
void path_filetype_set(file_t,FileType_t);
//ファイル名(文字列)からfile_tを取得する
file_t path_get_all(const char*); //Pathを含む
file_t path_get(const char*);

file_t directory_create(const char *name);
int directory_insert(file_t directory, file_t file);
int directory_registor(file_t); //Pathに登録する。
void directory_registor_init();

int shell_system(char* name);
int shell_system_s(int argc, char** argv);
#endif	/* SHELL_CORE_H */

