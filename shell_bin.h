#ifndef SHELL_BASIC_H
#define	SHELL_BASIC_H

//binディレクトリをrootに追加する。
void bin_registor();//制約　path_initが実行されていること。

//binディレクリトリコマンドを記述する。なおパイプは実装されていないので注意が必要である。
int bin_exit(int,char**);
int bin_echo(int,char**);
int bin_ls(int,char**);
int bin_pwd(int,char**);
int bin_tree(int,char**);
int bin_cd(int,char**);
int bin_system(int,char**);//廃棄予定
int bin_set(int,char**);
int bin_get(int,char**);

#endif	/* SHELL_BASIC_H */

