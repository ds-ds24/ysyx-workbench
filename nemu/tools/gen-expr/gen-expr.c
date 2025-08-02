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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char *p = buf;  
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format = 
"#include <stdio.h>\n"
"int main() { "
"  int result = %s; "
"  printf(\"%%d\", result); "
"  return 0; "
"}";
void reset_buffer(){
  p = buf;
  *p = '\0';
}
void gen(char c){
  *p++ = c;
  *p = '\0';
}
void gen_str(char *s){
  strcpy(p, s);
  p += strlen(s);
}
void gen_num(){
  int num = rand()%100;
   char str_num[10];
   sprintf(str_num,"%d",num);
   gen_str(str_num);
}
void gen_rand_op(char *op){
  char ops[] = "+-*/";
  *op = ops[rand()%4];
  gen(' ');
  gen(*op);
  gen(' ');
}
int choose(int n){
  return rand()% n;
} 
static char* gen_rand_expr(int depth) {
  char *start = p;
  if(depth >3){
    gen_num();
    return start;
  }
  switch (choose(3)) {
    case 0: gen_num();break;
    case 1:gen('(');gen_rand_expr(depth+1);gen(')');break;
    default:
      gen_rand_expr(depth+1);
      char op;
      gen_rand_op(&op);
      gen_rand_expr(depth +1);
      break;
     
  }
  return start;
}

int main(int argc, char *argv[]) {
  int seed = time(0); 
  srand(seed);
  int loop = 1;
  if (argc > 1) { 
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    reset_buffer();
    gen_rand_expr(0);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {
      i=i-1;
      continue;
    }
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%d %s\n", result, buf);
  }
  return 0;
}
