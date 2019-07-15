#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAXSIZE 40

/* retire un caractère dans une chaine */
void *removechar(char toremove, char *dest, char *src, int len)
{
   int i = 0;
   while(*src != 0 && i++ < MAXSIZE)
     if(*src != toremove)
	*dest++ = *src++;
     else
        src++;
   *dest = 0;
}


int main(int argc, char* argv[])
{
   char jsonstring[MAXSIZE], slabel[MAXSIZE], svalue[MAXSIZE], dlabel[MAXSIZE], dvalue[MAXSIZE];
   int found = 0;

   while(1){
      fgets(jsonstring, MAXSIZE-1, stdin);
      sscanf(jsonstring, "   %s : %s,", slabel, svalue);
      //printf("Label:%s, Value: %s\n", slabel, svalue);
      removechar('"', dlabel, slabel, MAXSIZE); 
      removechar('"', dvalue, svalue, MAXSIZE); 
      strcpy(svalue, dvalue);
      removechar(',', dvalue, svalue, MAXSIZE); 
      //printf("Label:%s, Value: %s\n", dlabel, dvalue);
      if (!strcmp(dlabel, "idx") && atoi(dvalue) == 835){
	   //printf("Found !!!!\n");
	   found = 1;
      }
      if (found == 1 && !strcmp(dlabel, "svalue1") ){
         //printf("Label:%s, Value: %s\n", dlabel, dvalue);
         printf("%s\n", dvalue);
	 fflush(stdout);
         found = 0;
      }
   }
}

