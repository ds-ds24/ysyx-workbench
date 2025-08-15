#include <am.h>
//#include <cstdarg>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *p=out;
  int count=0;
  for(int i=0;fmt[i]!='\0';i++){
    if(fmt[i] != '%'){
      *p++=fmt[i];
      count++;
    }
    else{
      i++;
      switch(fmt[i]){
        case('s'):{
          char *str=va_arg(args,char*);
          for(int j=0;str[i]!='\0';j++){
            *p++=str[j];
            count++;
          }
          break;
        }
        case('d'):{
          int num=va_arg(args,int);
          char temp[12];
          int temp_idx=0;
          if(num<0){
            *p++='-';
            count++;
            num =-num;
          }
          if(num==0){
            *p++='0';
            count++;
            break;
          }
          while(num>0){
            temp[temp_idx++]='0' +(num % 10);
            num /= 10;
            }
            for(int j=temp_idx-1;j>=0;j--){
              *p=temp[j];
              p++;
              count++;
            }
            break;
          }
          default:
            *p++ = '%';
            *p++ = fmt[i];
            count += 2;
            break;
        }
      }

    }
    *p = '\0';
    va_end(args);
    return count;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
