#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <wiringPi.h>

#define CODE_SIZE 32	// bit number of the key code
#define IR_PIN	22 	// GPIO 22
#define NB_KEYS	11

/* us to ns coef */
#define US2NS		850 //830
/* duration in us */
#define SPACE_ONE 	1687
//#define SPACE_ONE 	1568
#define SPACE_ZERO 	562
#define MARK 		562
//#define SPACE_ZERO 	522
//#define MARK 		522
#define HEADER_MARK 	9000
#define HEADER_SPACE 	4500
#define GAP 		60000
// code for penguino
#define KEY_ONOFF	0xF8FFC13E
#define KEY_MODE	0xF8FF837C
#define KEY_FAN		0xF8FF03FC
#define KEY_PLUS 	0xF8FF21DE
#define KEY_MINUS 	0xF8FF01FE
#define KEY_TIMER	0xF8FF817E
#define KEY_OSC		0xF8FFA15E
#define KEY_HOME	0xF8FF41BE
#define KEY_SILENT	0xF8FF619E
// code for my LG TV 
#define KEY_LGONOFF	0x20DF10EF
#define KEY_LG1		0x20DF8877
#define KEY_LG2		0x20DF48B7
#define KEY_LG3		0x20DFC837
#define KEY_LG4		0x20DF28D7
#define KEY_LG5		0x20DFA857
#define KEY_LG6 	0x20DF6897
#define KEY_LG7		0x20DFE817
#define KEY_LG8		0x20DF18E7
#define KEY_LG9		0x20DF9867
#define KEY_LG0		0x20DF08F7
#define KEY_LGVOLPLUS	0X20DF40BF
#define KEY_LGVOLMINUS	0x20DFC03F
#define KEY_LGPROGPLUS	0x20DF00FF
#define KEY_LGPROGMINUS	0x20DF807F

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
  "MA", KEY_LGONOFF,
  "1", KEY_LG1,
  "2", KEY_LG2,
  "3", KEY_LG3,
  "4", KEY_LG4,
  "5", KEY_LG5,
  "6", KEY_LG6,
  "7", KEY_LG7,
  "8", KEY_LG8,
  "9", KEY_LG9,
  "0", KEY_LG0,
  "V+",KEY_LGVOLPLUS,
  "V-",KEY_LGVOLMINUS,
  "P+",KEY_LGPROGPLUS,
  "P-",KEY_LGPROGMINUS
};

typedef struct {
  int domoticzcode;
  long keyvalue;
} DOMOTICZKEY;

DOMOTICZKEY pkeys[] = {
  0, KEY_ONOFF,
  -1, KEY_MODE, // not used in Domoticz
  10, KEY_FAN,
  20, KEY_PLUS,
  30, KEY_MINUS,
  -1, KEY_TIMER, // nont used in Domoticz
  40, KEY_OSC,
  50, KEY_HOME,
  60, KEY_SILENT,
};

DOMOTICZKEY lgkeys[] = {
  0, KEY_LGONOFF,
  10,KEY_LGPROGPLUS,
  20,KEY_LGPROGMINUS,
  30,KEY_LGVOLPLUS,
  40,KEY_LGVOLMINUS,
  50, KEY_LG1,
  60, KEY_LG2,
  70, KEY_LG3,
  80, KEY_LG4,
  90, KEY_LG5,
  100, KEY_LG6,
  110, KEY_LG7,
  120, KEY_LG8,
  130, KEY_LG9,
  140, KEY_LG0,
};

struct timespec req;
int us2ns = US2NS;

//struct time_t mytime;
/* transmission of the code (32 bits) */
void  sendcode(long code)
{
   int i;

   for(i=0; i<CODE_SIZE; i++){
      if (code < 0){
         digitalWrite(IR_PIN, HIGH);
         req.tv_nsec = MARK*us2ns;
         nanosleep(&req, NULL);
         digitalWrite(IR_PIN, LOW);
         req.tv_nsec = SPACE_ONE*us2ns;
         nanosleep(&req, NULL);
      }
      else {
         digitalWrite(IR_PIN, HIGH);
         req.tv_nsec = MARK*us2ns;
         nanosleep(&req, NULL);
         digitalWrite(IR_PIN, LOW);
         req.tv_nsec = SPACE_ZERO*us2ns;
         nanosleep(&req, NULL);
      }
      code <<=1 ;
   }
}

char *myctime(char *strtime)
{
   time_t t;

   t = time(NULL);
   strcpy(strtime, ctime(&t));
   strtime[strlen(strtime)-1] = 0;

   return strtime;
}

int main (int argc, char** argv)
{
   int repeat = 0, bursts = 1, domoticzmode = 0, domoticzcode;
   char key[10];
   int i, j, c, nbkeys = sizeof(pkeys)/sizeof(DOMOTICZKEY);
   long code;
   DOMOTICZKEY *p = pkeys;
   char t[80];

   /* default values */
   strcpy(key, "ONOFF");

   /* gzetting parameters on command call */
   while ((c = getopt(argc , argv, "dk:r:lhb:c:")) != -1)
      switch (c) {
      case 'd':
	 fprintf(stderr, "%s: Entering in domoticz mode\n", myctime(t));
         domoticzmode = 1;
	 break;
      case 'k':
         strcpy(key, optarg);
         break;
      case 'r':
         repeat = atoi(optarg);
         break;
      case 'l':
	 nbkeys = sizeof(lgkeys)/sizeof(DOMOTICZKEY);
	 p=lgkeys;
	 fprintf(stderr, "%s: Entering in LG TV mode\n", myctime(t));
	 break;
      case 'b':
	 bursts = atoi(optarg);
	 if(bursts <1) bursts = 1;
	 if(bursts > 10) bursts = 10;
	 fprintf(stderr, "%s: Entering in burst mode with %d burst(s)\n", myctime(t), bursts);
	 break;
      case 'c':
         us2ns = atoi(optarg);
         break;
      case 'h':
      default: /* '?' */
         fprintf(stderr, "Usage: %s [-d] [-l] [-k <KEYVALUE>] [-r <repeat value (integer)>  [-b <burst value (integer)>] [-c <timing coeficient (integer)>]\n", argv[0]);
	 fprintf(stderr, "When -d option is selected (domoticz mode) then -k option is ignored\n"); 
         exit(EXIT_FAILURE);
     }

   /* verify -b and -r options consistency */
   if ((bursts >1) && (repeat >1)){
      fprintf(stderr, "Usage : cannot use -r and -b options at the same time\n");
      exit(EXIT_FAILURE);
   }
   /* initialisations diverses */
   req.tv_sec = 0;
   /* WiringPi initialization */
   wiringPiSetupGpio() ;
   pinMode(IR_PIN, OUTPUT);
   fprintf(stderr, "%s: Nb keys: %d\n", myctime(t), nbkeys); 
   fprintf(stderr, "%s: Using timing coefficient :%d\n", myctime(t), us2ns);

   /* endless loop if in Domoticz mode otherwise on-shot only/si on est en mode domoticz on boucle sans fin, sinon c'est one-shot */
   do{
      //getting the code to transmit (manual mode)/obtenir le code à émettre (mode manuel)
      if(domoticzmode == 0){
         for(i = 0; i < sizeof(mkeys)/sizeof(MANUALKEY); i++)
            if(!strcmp(key, mkeys[i].keyname)){
               code = mkeys[i].keyvalue;
	       break;
      	    }
      }
      //getting the code to transmit in Domoticz mode/ obtenir le code à émettre mode Domoticz
      else {/*domoticsmode == 1*/
	 fscanf(stdin, "%d", &domoticzcode);
	 fprintf(stderr, "%s: Code domotique reçu : %d\n", myctime(t), domoticzcode);
         for(i = 0; i < nbkeys; i++)
            if(domoticzcode == p[i].domoticzcode){
               code = p[i].keyvalue;
	       break;
            }
      }

      if(i == nbkeys){
        fprintf(stderr, "%s: Key not found\n", myctime(t));
	if(domoticzmode == 1)
	   continue;
	else
	   exit(1);
      }

      fprintf(stderr, "%s: Emission du code 0x%08x\n", myctime(t), code);
for(j=0; j<bursts; j++){
      // transmitting the header
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = HEADER_MARK*us2ns;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);
      req.tv_nsec = HEADER_SPACE*us2ns;
      nanosleep(&req, NULL);

      /* transmitting the code itself  */
      sendcode(code);

      /* transmitting the stop bit  */
      digitalWrite(IR_PIN, HIGH);
      req.tv_nsec = MARK*us2ns;
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
      req.tv_nsec = MARK*us2ns;
      nanosleep(&req, NULL);
      digitalWrite(IR_PIN, LOW);
      usleep(96190);
      }

      // gap before tranmitting another code
      req.tv_nsec = GAP*us2ns;
      nanosleep(&req, NULL);
} // end of burst loop
   } while(domoticzmode == 1);
   return 0;
}
