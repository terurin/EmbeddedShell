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
#include "shell_core.h"
//やっと本格的な操作
file_t execute_create(const char* name, int(*)(int, char**)); //そのまま実行可能なものつまり関数。
file_t integer_create(const char* name, int* ptr, Access_t acc);
file_t float_create(const char* name,float* ptr,Access_t acc);


int shell_error();

void shell_init();

#endif	/* SHELL_H */

