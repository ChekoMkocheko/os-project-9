#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
int main(){
uint64_t num; 
num = 100000;
uint64_t i = 0;
FILE *fp;
char* str = "string";
int x = 0;

fp=fopen("test.txt", "w");
if(fp == NULL)
    exit(-1);
while(i<num){
x++;
fprintf(fp, "This is a string which is written to a file\n");
fprintf(fp, "The string has %d words and keyword %s\n", x, str);
i++;
}
fclose(fp);
return 0;
}
