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
#include "debug.h"
#include "macro.h"
#include "memory/paddr.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum { 
  TK_NOTYPE = 256, TK_EQ,TK_NUM,TK_NOEQ,TK_AND,TK_XNUM,DEREF,TK_RE,TK_F

  /* TODO: Add more token types */

};

static struct rule { 
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},
  {"\\(",'('},
  {"\\)",')'},
  {"&&",TK_AND},
  {"==", TK_EQ},
  {"!=",TK_NOEQ},
  {"0x[0-9a-fA-F]+",TK_XNUM},
  {"[0-9]+",TK_NUM},
  {"\\$[a-zA-Z0-9]+",TK_RE},
  {"\\+", '+'},         // plus
  {"\\-",'-'},
  {"\\*",'*'},
  {"/",'/'},
  


};

#define NR_REGEX ARRLEN(rules)  

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};  
static int nr_token __attribute__((used))  = 0;  

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') { 
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_AND:
          case TK_EQ:
          case TK_NOEQ:
          case '$':
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
            tokens[nr_token].type=rules[i].token_type;
            tokens[nr_token].str[0]=rules[i].token_type;
            tokens[nr_token].str[1]='\0';
            nr_token++;
            break;
          case TK_NUM:
          case TK_XNUM:
            tokens[nr_token].type=rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start,substr_len);
            tokens[nr_token].str[substr_len]='\0';
            nr_token++;
            break;
            case TK_NOTYPE:
              continue;
          default: 
            assert(0);
        }
        break;
      }
    }
    for(int i=0;i<nr_token;i++){
      if(tokens[i].type == '*'){
        if(i == 0||tokens[i-1].type == '+'||tokens[i-1].type == '-'||tokens[i-1].type == '('||tokens[i].type==TK_EQ||tokens[i].type==TK_NOEQ||tokens[i-1].type==TK_AND){
          tokens[i].type = DEREF;
        }
      }
    }
    for(int i=0;i<nr_token;i++){
      if(tokens[nr_token].type=='-'){
        if(i==0||tokens[i-1].type == '('||tokens[i-1].type=='+'||tokens[i-1].type=='-'||tokens[i-1].type==TK_AND||tokens[i-1].type==TK_EQ||tokens[i-1].type==TK_NOEQ){
          tokens[i].type = TK_F;
        }
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
static bool check_parentheses(int p,int q){
  if(tokens[p].type != '('||tokens[q].type != ')'){
    return false;
  }
  int balance = 0;
  for(int i=p;i<=q;i++){
    if(tokens[i].type =='(') balance++;
    else if(tokens[i].type == ')') balance--;
    if(balance<0) return false;
  }
  if(balance==0){
    return true;
  }
  else{
    return false;
  }
}

int eval(int p,int q) {
    if(p>q){
      printf("其实位置大于末位\n");
      return 0;
    }
    else if(p==q){
      char *next_op;
      if(tokens[p].type==TK_XNUM) return strtol(tokens[p].str,&next_op,16);
      else if(tokens[p].type==TK_NUM) return strtol(tokens[p].str,&next_op,10);
      else if(tokens[p].type==TK_RE) {
        bool success;
        word_t isa_num = isa_reg_str2val(tokens[p].str,&success);
        return isa_num;
      }
      else {
        printf("输入错误\n");
        return 0;
      }
    }
    else if(check_parentheses(p,q)==true){
      return eval(p+1,q-1);
    }
    else{
      int op=-1;
      int checkop=0;
      for(int i=q;i>=p;i--){
        if(tokens[i].type == '(') checkop++;
        else if(tokens[i].type == ')') checkop--;
        if(checkop != 0) continue;
        if(tokens[i].type == TK_AND){
          op=i;
          break;
        }
      }
      if(op==-1){
        for(int i=q;i>=p;i--){
          if(tokens[i].type == '(') checkop++;
          else if(tokens[i].type == ')') checkop--;
          if(checkop != 0) continue;
          if(tokens[i].type == TK_EQ||tokens[i].type==TK_NOEQ) {
            op=i;
            break;
          }
        }
      }
      if(op==-1){
        for(int i=q;i>=p;i--){
          if(tokens[i].type == '(') checkop++;
          else if(tokens[i].type == ')') checkop--;
          if(checkop != 0) continue;
          if(tokens[i].type == DEREF) {
            op=i;
            break;
          }
        }
      }
      if(op==-1){
        for(int i=q;i>=p;i--){
          if(tokens[i].type == '(') checkop++;
          else if(tokens[i].type == ')') checkop--;
          if(checkop != 0) continue;
          if(tokens[i].type == '+'||tokens[i].type=='-') {
            op=i;
            break;
          }
        }
      }
      if(op==-1){
        for(int i=q;i>=p;i--){
          if(tokens[i].type == '(') checkop++;
          else if(tokens[i].type == ')') checkop--;
          if(checkop != 0) continue;
          if(tokens[i].type == '*'||tokens[i].type=='/') {
            op=i;
            break;
          }
        }
      }
      if(op==-1){
        for(int i=q;i>=p;i--){
          if(tokens[i].type == '(') checkop++;
          else if(tokens[i].type == ')') checkop--;
          if(checkop != 0) continue;
          if(tokens[i].type == TK_F) {
            op=i;
            break;
          }
        }
      }
    
      int val1=eval(p,op-1);
      int val2=eval(op+1,q);

      switch(tokens[op].type){
        case TK_AND:return val1 && val2;
        case TK_EQ: return val1 == val2;
        case TK_NOEQ: return val1 != val2;
        case DEREF: return paddr_read(val2, 4);
        case TK_F: return val2*(-1);
        case '+':return val1 + val2;
        case '-':return val1 - val2;
        case '*':return val1 * val2;
        case '/':
          if(val2==0) {
            printf("/0错误");
            return 0;
          }
          return val1 / val2;
        default: assert(0);
      }
    }
  }


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0; 
  }

  /* TODO: Insert codes to evaluate the expression. */
  else if(nr_token==0){
    *success = false;
    return 0;
  }
  else if(check_parentheses(0,nr_token-1)){
    *success = true;
    return eval(1,nr_token-2);
  }
  else {
    *success = true;
    return eval(0,nr_token-1);
  }

}

