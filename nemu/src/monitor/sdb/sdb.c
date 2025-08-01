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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "common.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory/paddr.h>
#include <threads.h>
#include <uchar.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	//exit(0);
  //NEMUState nemu_state = { .state = NEMU_QUIT };
  nemu_state.state=NEMU_QUIT;
  return -1;
}


static int cmd_si(char *args) {
	int n = (args != NULL) ? atoi(args) : 1;
	cpu_exec(n);
	printf("Step execute N=%d\n",n);
	return 0;
}

static int cmd_w(char *args){
  if(!args){
    printf("Usage: w EXPR\n");
     return 0;
  }
  bool success;
  word_t us = expr(args,&success);
  if(success){
    WP* wp = add_wp(  args);
    printf("Hardware watchpoint %d: %s. value :%d)\n",wp->NO,wp->expr_str,us);
    return 0;
  }
  else {
    printf("表达式错误\n");
    return 0;
  }
}

static int cmd_info(char *args) {
	if(strcmp(args,"r")==0){
		isa_reg_display();
	}
  if(strcmp(args,"w")==0){
    info_watchpoint();
  }
	return 0;
}
static int cmd_d(char *args){
  int no = atoi(args);
  def_wp(no);
  return 0;
}
static int cmd_p(char *args){
  init_regex();
  bool success;
  word_t endnum = expr(args,&success);
  if(success){
    printf("0x%08x\n",endnum);
  }
  if(!success){
    printf("括号不匹配");
  }
  return 0;
}

static int cmd_x(char *args){
  int len=0;
  paddr_t addr = 0;
  sscanf(args,"%d %x",&len,&addr);
  printf("以%x为起始地址打印%d内存数据:\n",addr,len);
  int i=0;
  for(i=0;i<len;i++){
    printf("%08x: %08x\n",addr,paddr_read(addr,4));
    addr += 4;
  }
  return 0;
}

static int cmd_fp(char *args){
  FILE *fp=fopen("/home/ds24/ysyx-workbench/nemu/tools/gen-expr/input","r");
  char buf[100];
  char read[1000][100];
  char read1[1000][50];
  char read2[1000][50];
  //bool success;
  int i=0;    
  while (fgets(buf,sizeof(buf),fp) != NULL){
    strcpy(read[i],buf);
    size_t len = strlen(read[i]);
    if(len > 0 && read[i][len-1]=='\n'){
      read[i][len-1] = '\0';
    }
    char *token = strtok(read[i]," ");
    strcpy(read1[i],token);
    char *expr_str = strtok(NULL,"\0");
    //printf("%s\n",expr_str);
    strcpy(read2[i],expr_str);
    //word_t endnum =expr(expr_str,&success);
    //printf("%s,%u\n",read1[i],endnum);
    i++;
  }
  int j=0;
  while(j<=500){
    printf("%s\n",read2[j]);
    j++;
  }
  
  //q
  // printf("%s\n",read[0]);
  // printf("%s\n%s",read1[0],read2[0]);
  //expr(fp, &success);
  fclose(fp);
  // bool success;
  // while(read2[i] != "\0"){
  //   word_t endnum = expr(read2,&success);
  //   printf("%s,%d",read1[i],endnum);
  // }

  return 0;
}

static int cmd_help(char *args);


static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si","Let the program excute N instuctions in a single step and the suspend execution",cmd_si},
	{"info","Print registers",cmd_info},
  {"x","Scan memory",cmd_x},
  {"p","Expression evaluation",cmd_p},
  {"w","Add watchpoints",cmd_w},
  {"d","Delete watchpoints",cmd_d},
  {"fp","Read input.text",cmd_fp}

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
