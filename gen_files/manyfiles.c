#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#define filenametemplate "many_test_%d.txt"

int main(){
uint64_t num; 
num = 1664;
uint64_t i = 0;
FILE *fp;
while(i < num){
char filename[30];
sprintf(filename, filenametemplate, i);
fp=fopen(filename, "w");
    if(fp == NULL){
        exit(-1);
    }
    fprintf(fp, "This is the content of the file\n");
    fprintf(fp, "This is file num: %d\n", i);
    i++;
 fclose(fp);
}

return 0;
}
