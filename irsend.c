#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <wiringPi.h>

#define CODE_SIZE 32	// bit number of the key code

#define IR_PIN	22 // GPIO 22

/* duration in ns */
#define SPACE_ONE 	1687
#define SPACE_ZERO 	562
#define MARK 		562
#define HEADER_MARK 	9000
#define HEADER_SPACE 	4500
#define GAP 		60000

#define NB_KEYS		9
#define KEY_ONOFF	0xF8FFC13E
#define KEY_MODE	0xF8FF837C
#define KEY_FAN		0xF8FF03FC
#define KEY_PLUS 	0xF8FF21DE
#define KEY_MINUS 	0xF8FF01FE
#define KEY_TIMER	0xF8FF817E
#define KEY_OSC		0xF8FFA15E
#define KEY_HOME	0xF8FF41BE
#define KEY_SILENT	0xF8FF619E
// code for my LG TV (for test purpose only)
#define KEY_LGMA	0x20DF10EF
#define KEY_LGONE	0x20DF8877
#define KEY_LGTWO	0x20DF48B7



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
  -1, KEY_MODE, // not used in Domoticz
  10, KEY_FAN,
  20, KEY_PLUS,
  30, KEY_MINUS,
  -1, KEY_TIMER, // nont used in Domoticz
  40, KEY_OSC,
  50, KEY_HOME,
  60, KEY_SILENT 
};

struct timespec req;

/* transmission of the code (32 bits) */
void  sendcode(long code)
{
   int i;

   for(i=0; i<CODE_SIZE; i++){
      if (code < 0){
         digitalWrite(IR_PIN, HIGH);
         req.tv_nsec = MARK*1000;
         nanosleep(&req, NULL);
         digitalWrite(IR_PIN, LOW);
         req.tv_nsec = SPACE_ONE*1000;
         nanosleep(&req, NULL);
      }
      else {
         digitalWrite(IR_PIN, HIGH);
         req.tv_nsec = MARK*1000;
         nanosleep(&req, NULL);
         digitalWrite(IR_PIN, LOW);
         req.tv_nsec = SPACE_ZERO*1000;
         nanosleep(&req, NULL);
      }
      code <<=1 ;
   }
}

int main (int argc, char** argv)
{
   int repeat = 0, domoticzmode = 0, domoticzcode;
   char key[10];
   int i, c;
   long code;


   /* default values */
   strcpy(key, "ONOFF");

   /* gzetting parameters on command call */
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
   /* WiringPi initialization */
   wiringPiSetupGpio() ;
   pinMode(IR_PIN, OUTPUT);

   /* endless loop if in Domoticz mode otherwise on-shot only/si on est en mode domoticz on boucle sans fin, sinon c'est one-shot */
   do{
      //gettin the code to transmit (manual mode)/obtenir le code à émettre (mode manuel)
      if(domoticzmode == 0){
         for(i = 0; i < NB_KEYS; i++)
            if(!strcmp(key, mkeys[i].keyname)){
               code = mkeys[i].keyvalue;
	       break;
      	    }
      }
      //getting the code to transmit in Domoticz mode/ obtenir le code à émettre mode Domoticz
      else {/*domoticsmode == 1*/
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

      // transmitting the header
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = HEADER_MARK*1000;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);
      req.tv_nsec = HEADER_SPACE*1000;
      nanosleep(&req, NULL);

      /* transmitting the code itself  */
      sendcode(code);

      /* transmitting the stop bit  */
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = MARK*1000;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);

      //repeat code  if requested (option -r)
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

      // gap before tranmitting another code
      req.tv_nsec = GAP*1000;
      nanosleep(&req, NULL);

   } while(domoticzmode == 1);
   return 0;
}
