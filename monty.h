#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <locale.h>

#define ERRORSTR(s) printf(failStr, s)
#define ERRORRET(t, s) printf("%s: %s\n", t, s); ERRORSTR(t); return 0

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define SETCURSORLEFT "\033[10D"

#define MAXCORES 64 // Yeah 64 is probably too many (I think now... till iunno 30 years from now I look back and think "64? a modern cpu is measured in kilocores wtf is this?".

// Colors
#define C_G "\e[92m"
#define C_R "\e[91m"
#define C_RST "\e[m"

// Constant strings
const char *ContestantChoiceStr[] = {"\e[94mto stick with door\e[m", "\e[93mto change to door\e[m"};
const char *failStr = "Use %s -h for more information\n";
const char *helpStr = \
		"Usage: %s [OPTION]\n"\
		"Monty Hall game simulation\n\n"\
		"  -p\t\tSpecify number of decimal points displayed (between 2 and 10)\n"\
		"  -d\t\tDelay each game by number of ms\n"\
		"  -t\t\tManually set number of threads to use in multithreaded mode\n"\
		"  -T\t\tStop games after number of seconds\n"\
                "  -r\t\tManually set status refresh rate in ms in multithreaded mode\n"\
		"  -s\t\tSingle threaded verbose mode\n"\
		"  -S\t\tShow thousands seperators\n"\
		"  -g\t\tWait for enter at each round when in single threaded mode, no effect otherwise\n"\
		"  -a\t\tNo ANSI cursor controls, show ALL games (no effect in multithreaded mode)\t\t\n"\
		"  -h\t\tDisplays this help informaion\n";

// Globals
int killtime=0;
int verbose=0;
int noAnsi=0;
int stop=0;
int refreshRate=100;
int gameDelay=0;
int numDecPoints=2;

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

// Global Thread Table
GameThread *gameThreadTable[MAXCORES] = {0}; 

