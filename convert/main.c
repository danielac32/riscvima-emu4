

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include "melf.h"



int replacestr(char *line, const char *search, const char *replace)
{
   int count;
   char *sp; // start of pattern

   //printf("replacestr(%s, %s, %s)\n", line, search, replace);
   if ((sp = strstr(line, search)) == NULL) {
      return(0);
   }
   count = 1;
   int sLen = strlen(search);
   int rLen = strlen(replace);
   if (sLen > rLen) {
      // move from right to left
      char *src = sp + sLen;
      char *dst = sp + rLen;
      while((*dst = *src) != '\0') { dst++; src++; }
   } else if (sLen < rLen) {
      // move from left to right
      int tLen = strlen(sp) - sLen;
      char *stop = sp + rLen;
      char *src = sp + sLen + tLen;
      char *dst = sp + rLen + tLen;
      while(dst >= stop) { *dst = *src; dst--; src--; }
   }
   memcpy(sp, replace, rLen);

   count += replacestr(sp + rLen, search, replace);

   return(count);
}

 



int main(int argc, char const *argv[])
{
	
    char *path;
    path = malloc(100);
    strcpy(path,argv[1]);


    FILE* fpSrc = fopen(path, "r");
    if(fpSrc==NULL){
        printf("error open file %s\n",path );
    }
    unsigned long lsize = 0;
    char *p;
    fseek(fpSrc, 0, SEEK_END);
    lsize = ftell(fpSrc);
    rewind(fpSrc);
    p=(char*)malloc(lsize);
    fread(p, sizeof(char), lsize, fpSrc);
    fclose(fpSrc);

    exec_img ximg;
    program_img point;
    point.img = (void*)p;

    int r = load_elf(&point, &ximg);
    printf("%d %d\n", r,ximg.end-ximg.start);
    
    remove(path);
    replacestr(path,".elf",".bin");
    fpSrc = fopen(path, "w");
    if(fpSrc==NULL){
        printf("error open file %s\n",path );
    }

    fwrite(ximg.start, sizeof(char), ximg.end-ximg.start, fpSrc);
    fclose(fpSrc);

    free(p);
    free(path);
	return 0;
}

