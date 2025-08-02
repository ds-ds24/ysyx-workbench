/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>
#include <stdbool.h>

word_t expr(char *e, bool *success);
bool str_condition(const char *str_expr);

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  word_t value;
  char *expr_str;
  bool has_condition;
  word_t right_value;
} WP;


WP* add_wp(const char* str);
bool def_wp(int no);
bool check_wp();
void info_watchpoint();

#endif
