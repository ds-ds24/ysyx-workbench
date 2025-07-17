#include <stdio.h>

int main(){
    int a=0,b=1,c=0;
    while(c<=10){
        c = a+b;
        a = b;
        b++;
    }
    return 0;
}