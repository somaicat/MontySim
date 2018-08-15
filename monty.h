#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <locale.h>
#include <dlfcn.h>

#define ERRORSTR(s) printf(failStr, s)
#define ERRORRET(t, s) printf("%s: %s\n", t, s); ERRORSTR(t); return 0

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define SETCURSORLEFT "\033[10D"
#define CLEARLINE "                                                                               \033[80D"

#define MAXCORES 64 // Yeah 64 is probably too many (I think now... till iunno 30 years from now I look back and think "64? a modern cpu is measured in kilocores wtf is this?".

// Colors
#define C_G "\e[92m"
#define C_R "\e[91m"
#define C_RST "\e[m"
#define C_WBG "\e[30;47m"
#define C_DOSBG "\e[1;37;44m"
#define C_DOSFG "\e[31;47m"

// Constant strings
const static char *ContestantChoiceStr[] = {"\e[94mto stick with door\e[m", "\e[93mto change to door\e[m"};
const static char *failStr = "Use %s -h for more information\n";
const static char *helpStr = \
		"Usage: %s [OPTION]\n"\
		"Monty Hall game simulation\n\n"\
		"  -p <points>\tSpecify number of decimal points displayed (between 2 and 10)\n"\
		"  -d <delay>\tDelay each game by number of ms\n"\
		"  -S\t\tShow thousands seperators\n"\
		"  -h\t\tDisplays this help informaion\n"\
		"\n--------- Multithreaded options\n"\
		"  -t <threads>\tManually set number of threads to use\n"\
		"  -T <timeout>\tStop games after number of seconds\n"\
		"  -A\t\tNo ANSI background colors (foreground colors still exist if supported)\n"\
		"  -n\t\tDon't load any external UI libraries\n"\
		"\n--------- Singlethreaded options\n"\
		"  -s\t\tSingle threaded (verbose) mode\n"\
		"  -g\t\tWait for enter at the end of each game\n"\
		"  -a\t\tNo ANSI cursor controls, show ALL games\n";

// Globals //Note: This will need modification if we ever switch to more code files.
extern int killtime;
extern int verbose;
extern int noAnsi;
extern int stop;
extern int refreshRate;
extern int gameDelay;
extern int numDecPoints;
extern int timer;
extern int nCpus;
extern char *bgColor;
extern time_t startTime;

// Structures
typedef struct GameScore { 
  unsigned long long numWonWSwitch;
  unsigned long long numWonWoSwitch;
  unsigned long long numLostWSwitch;
  unsigned long long numLostWoSwitch;
  float percentWonWSwitch;
  float percentWonWoSwitch;
} GameScore;

typedef struct GameThread {
  pthread_t thread;
  GameScore score;
  unsigned int seed; 
} GameThread;

// Functions
extern void IntOutputLoop();

// Global Thread Table
extern GameThread *gameThreadTable[MAXCORES]; 

