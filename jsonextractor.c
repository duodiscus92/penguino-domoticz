#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* idx of the switch virtual device on domotics server */
#define IDX 835

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
   int c, found = 0, idx=IDX;

   /* gzetting parameters on command call */
   while ((c = getopt(argc , argv, "i:")) != -1)
      switch (c) {
      case 'i':
         idx = atoi(optarg);
         break;
      case 'h':
      default: /* '?' */
         fprintf(stderr, "Usage: %s  [-i <IDX value> ]\n", argv[0]);
         exit(EXIT_FAILURE);
     }

   while(1){
      fgets(jsonstring, MAXSIZE-1, stdin);
      //printf("%s\n", jsonstring);
      sscanf(jsonstring, "   %s : %s,", slabel, svalue);
      //printf("Label:%s, Value: %s\n", slabel, svalue);
      removechar('"', dlabel, slabel, MAXSIZE); 
      removechar('"', dvalue, svalue, MAXSIZE); 
      strcpy(svalue, dvalue);
      removechar(',', dvalue, svalue, MAXSIZE); 
      //printf("Label:%s, Value: %s\n", dlabel, dvalue);
      if (!strcmp(dlabel, "idx") && atoi(dvalue) == idx){
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

