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

#include "common.h"
#include "isa.h"
#include "sdb.h"
#include "utils.h"
#include <stdbool.h>


#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr_str = NULL;
    wp_pool[i].has_condition = false;
  }

  head = NULL;
  free_ = wp_pool;
}


WP *add_wp_condition(const char *str_expr){
  if(free_ == NULL) assert(0); 
  WP *wp = free_;
  free_ = free_->next;
  char *eq_pos = strstr(str_expr, "==");
  if (eq_pos) {
    int var_len = eq_pos - str_expr;
    char *var = malloc(var_len + 1);
    if(!var) return false;
    strncpy(var, str_expr, var_len);
    (var)[var_len] = '\0';
    wp->expr_str = strdup(var);
    free(var);

    bool success;
    word_t right = expr(eq_pos + 2, &success);
    wp->right_value = right;
    wp->has_condition = true;
    wp->next = head;
    head = wp;
    return wp;
  } else {
    wp->has_condition = false;
    wp->expr_str = strdup(str_expr);
    wp->right_value = 0;
    bool success;
    wp->value = expr(wp->expr_str,&success);
    wp->next = head;
    head = wp;
    return wp;
  }
}


bool def_wp(int no){
  WP* prev = NULL;
  WP* curr = head;
  while(curr != NULL && curr->NO != no){
    prev = curr;
    curr = curr->next;
  }
  if(curr == NULL) {
    printf("找不到监视点\n");
    return false;
  }
  if(prev==NULL) head = curr->next;
  else prev->next = curr->next;
  free(curr->expr_str);
  curr->next = free_;
  free_ = curr;
  return true;
}
bool check_wp(){
  WP* curr = head;
  bool FT = false;
  while(curr != NULL){
    bool success;
    word_t new_value = expr(curr->expr_str,&success);
    if (curr->has_condition) {
      if (curr->right_value == new_value) {
        printf("监视点%d %s触发:0x%08x\n", curr->NO, curr->expr_str,new_value);
        nemu_state.state = NEMU_STOP;
        FT = true;
      }
    }
    else{
      if(new_value != curr->value){
        printf("监视点%d %s变化:从0x%08x变为0x%08x\n",curr->NO,curr->expr_str,curr->value,new_value);
        curr->value = new_value;
        nemu_state.state = NEMU_STOP;
        FT = true;
      }
    }
    curr = curr->next;
  }
  return FT;
}
void info_watchpoint(){
  WP* wp = head;
  if(head == NULL){
    printf("NO watchpoints.\n");
  }
  else{
    while(wp != NULL){
      if(strcmp(wp->expr_str,"$pc")==0){
        printf("监视点%d %s的:0x%08x\n",wp->NO,wp->expr_str,cpu.pc);
        wp = wp->next;
      }
      else{
        printf("监视点%d %s的值:0x%08x\n",wp->NO,wp->expr_str,wp->value);
        wp = wp->next;
      }
    }
  }
}

