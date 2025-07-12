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

#include "debug.h"
#include "macro.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum {                                                                  //特殊规则
  TK_NOTYPE = 256, TK_EQ,TK_NUM,

  /* TODO: Add more token types */

};

static struct rule {                                                   //规则结构
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\-",'-'},
  {"\\*",'*'},
  {"/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"[0-9]+",TK_NUM},



};

#define NR_REGEX ARRLEN(rules)                          //根据rules来确定NR_REGEX的大小

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {//初始化正则表达式，该函数确保所有预定义的正则表达式规则在调用之前进行正确的编译，若编译失败则立即终止并提供详细错误信息。  
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

typedef struct token {                              //元字符结构体
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};     //定义32个Token结构体类型的数组
static int nr_token __attribute__((used))  = 0;    //__attribute__((used))告诉编译器即使数组没有被显示使用，也不要优化掉它。这个在生成静态数据或库时特别有用

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {                   //识别并将token储存到数组中
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

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");//用^精确标记错误的地方
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
      return atoi(tokens[p].str);
    }
    else if(check_parentheses(p,q)==true){
      return eval(p+1,q-1);
    }
    else{
      int op = -1,op1=-1,op2=-1;
      int checkop=0;
      for(int i=p;i<q;i++){
        if(tokens[i].type == '(') checkop++;
        else if(tokens[i].type == ')') checkop--;
        if(checkop != 0){
          continue;
        }
        if(tokens[i].type == '+'||tokens[i].type == '-'){
          op1=i;
          continue;
        }
        else if(tokens[i].type == '*'||tokens[i].type == '/'){
          op2 = i;
          continue;
        }
      }
      if(op1>=0) op = op1;
      else op=op2;

      int val1=eval(p,op-1);
      int val2=eval(op+1,q);

      switch(tokens[op].type){
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
  else if(!check_parentheses(0, nr_token)){
    *success = false;
    return 1;
  }
  else if(check_parentheses(0,nr_token-1)){
    *success = true;
    return eval(0,nr_token-1);
  }
  else {
    *success = false;
    return 0;
  }

}


