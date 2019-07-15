#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <wiringPi.h>

#define CODE_SIZE 32

#define IR_PIN	22

/*
#define MARK_ONE  580
#define MARK_ZERO 1600
#define SPACE 580
#define HEADER_MARK 8000
#define HEADER_SPACE 4000
#define GAP 600000
#define KEY_PLUS 0x0700DE21
*/

#define SPACE_ONE 1687 /*1500*/
#define SPACE_ZERO /*600*/562
#define MARK /*600*/ /*500*/562
#define HEADER_MARK 9000
#define HEADER_SPACE 4500
#define GAP 60000

#define NB_KEYS		9
#define KEY_ONOFF	0xF8FFC13E
#define KEY_MODE	0xF8FF837C /*0X07007C83*/
#define KEY_FAN		0xF8FF03FC /*0X0700FC03*/
#define KEY_PLUS 	0xF8FF21DE /*0x0700DE21*/
#define KEY_MINUS 	0xF8FF01FE /*0x0700FE01*/
#define KEY_TIMER	0xF8FF817E /*0x07007E81*/
#define KEY_OSC		0xF8FFA15E /*0x07005EA1*/
#define KEY_HOME	0xF8FF41BE
#define KEY_SILENT	0xF8FF619E /*0X07009E61*/
// code for LG TV (for test purpose)
#define KEY_LGMA	0x20DF10EF
#define KEY_LGONE	0x20DF8877
#define KEY_LGTWO	0x20DF48B7


//struct timespec {
//   time_t tv_sec;        /* seconds */
//   long   tv_nsec;       /* nanoseconds */
//} req;

typedef enum {ONOFF, MODE, FAN, PLUS, MINUS, TIMER, OSC, HOME, SILENT} KEYS;

typedef struct {
  char keyname[10];
  long keyvalue;
} MANUALKEY;

MANUALKEY mkeys[] = {
  "ONOFF", KEY_ONOFF, 
  "MODE", KEY_MODE,
  "FAN", KEY_FAN,
  "PLUS", KEY_PLUS,
  "MINUS", KEY_MINUS,
  "TIMER", KEY_TIMER,
  "OSC", KEY_OSC,
  "HOME", KEY_HOME,
  "SILENT", KEY_SILENT,
  "MA", KEY_LGMA,
  "ONE", KEY_LGONE,
  "TWO", KEY_LGTWO 
};

typedef struct {
  int domoticzcode;
  long keyvalue;
} DOMOTICZKEY;

DOMOTICZKEY dkeys[] = {
  0, KEY_ONOFF, 
  -1, KEY_MODE, // non utilisé en mode domotique
  10, KEY_FAN,
  20, KEY_PLUS,
  30, KEY_MINUS,
  -1, KEY_TIMER, // non utilisé en mode domotique
  40, KEY_OSC,
  50, KEY_HOME,
  60, KEY_SILENT 
};

struct timespec req;

void  sendcode(long code)
{
   int i;

   for(i=0; i<CODE_SIZE; i++){
      if (code < 0){
         //printf("1");
         // it is a 1
         digitalWrite(IR_PIN, HIGH);
         //printf("|");
         req.tv_nsec = MARK*1000;
         nanosleep(&req, NULL);
         //printf("~");
         digitalWrite(IR_PIN, LOW);
         //printf("|");
         req.tv_nsec = SPACE_ONE*1000;
         nanosleep(&req, NULL);
         //printf("_");
      }
      else {
         //printf("0");
         // it is a 0
         digitalWrite(IR_PIN, HIGH);
         //printf("|");
         req.tv_nsec = MARK*1000;
         nanosleep(&req, NULL);
         //printf("~");
         digitalWrite(IR_PIN, LOW);
         //printf("|");
         req.tv_nsec = SPACE_ZERO*1000;
         nanosleep(&req, NULL);
         //printf("__");
      }
      code <<=1 ;
   }
   //printf("\n");
}

int main (int argc, char** argv)
{
   int repeat, domoticzmode = 0, domoticzcode;
   char key[10];
   int i, c;
   long code;


   /* valeur par défaut des options */
   repeat = 1;
   strcpy(key, "ONOFF");

   /* récupération des options de la ligne de commande */
   while ((c = getopt(argc , argv, "dk:r:h")) != -1)
      switch (c) {
      case 'd':
	 fprintf(stderr, "Entering in domoticz mode\n");
         domoticzmode = 1;
	 break;
      case 'k':
         strcpy(key, optarg);
         break;
      case 'r':
         repeat = atoi(optarg);
         break;
      case 'h':
      default: /* '?' */
         fprintf(stderr, "Usage: %s [-d] [-k <KEYVALUE>] [-r <repeat value (integer)>]\n", argv[0]);
	 fprintf(stderr, "When -d option is selected (domoticz mode) then -k option is ignored\n"); 
         exit(EXIT_FAILURE);
     }

   /* initialisations diverses */
   req.tv_sec = 0;
   wiringPiSetupGpio() ;
   pinMode(IR_PIN, OUTPUT);

   /* si on est en mode domoticz on boucle sans fin, sinon c'est one-shot */
   do{
      //obtenir le code à émettre (mode manuel)
      if(domoticzmode == 0){
         for(i = 0; i < NB_KEYS; i++)
            if(!strcmp(key, mkeys[i].keyname)){
               code = mkeys[i].keyvalue;
	       break;
      	    }
      }
      //obtenir le code à émettre mode domoticz
      else {/*domoticsmode == 1*/
	 //fgets(key, 10, stdin);
	 //fprintf(stderr, "Lecture code domotique sur stdin:%s\n", key);
	 //domoticzcode = atoi(key);
	 fscanf(stdin, "%d", &domoticzcode);
	 fprintf(stderr, "Code domotique reçu : %d\n", domoticzcode);
         for(i = 0; i < NB_KEYS; i++)
            if(domoticzcode == dkeys[i].domoticzcode){
               code = dkeys[i].keyvalue;
	       break;
            }
      }

      if(i == NB_KEYS){
        fprintf(stderr, "Key not found\n");
	if(domoticzmode == 1)
	   continue;
	else
	   exit(1);
      }

      fprintf(stderr, "Emission du code 0x%08x\n", code);
      //header
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = HEADER_MARK*1000;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);
      req.tv_nsec = HEADER_SPACE*1000;
      nanosleep(&req, NULL);

      /* emettre le code infrarouge */
      sendcode(code);

      //stop
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = MARK*1000;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);

      //repeat code 
      usleep(36000);
      for(i=0; i<repeat; i++){
      digitalWrite(IR_PIN, HIGH);
      usleep(9000);
      digitalWrite(IR_PIN, LOW);
      usleep(2250);
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = MARK*1000;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);
      usleep(96190);
      }

      // gap
      //digitalWrite(IR_PIN, LOW);
      req.tv_nsec = GAP*1000;
      nanosleep(&req, NULL);

   } while(domoticzmode == 1);
   return 0;
}
