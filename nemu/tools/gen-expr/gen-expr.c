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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char *p = buf;  
static int chuling=0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format = 
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
void reset_buffer(){
  p = buf;
  *p = '\0';
}
void gen(char c){
  *p++ = c;
  *p = '\0';
  chuling++;
}
void gen_str(char *s){
  strcpy(p, s);
  p += sizeof(s);
  chuling++;
}
void gen_num(){
  int num = rand()%100;
   char str_num[10];
   sprintf(str_num,"%x",num);
   gen_str(str_num);
   chuling++;
}
void gen_rand_op(){
  char ops[] = "+-*/";
  gen(' ');
  gen(ops[rand()%4]);
  gen(' ');
  chuling += 3;
}
int choose(int n){
  return rand()% n;
} 
static void gen_rand_expr() {
  switch (choose(3)) {
    case 0: gen_num();break;
    case 1:gen('(');gen_rand_expr();gen(')');break;
    default:gen_rand_expr(); gen_rand_op();gen_rand_expr();break;
  
  }
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
    int skip = 0;
    reset_buffer();
    gen_rand_expr();
    for(int i=0; i<chuling;i++){
      if(buf[i]=='/'&&buf[i+1]=='0'){
        printf("除0操作去除\n");
        skip =1;
      }
    }
    if(skip) break;
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
