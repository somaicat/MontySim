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

// Globals 
extern volatile sig_atomic_t killtime;
extern int verbose;
extern int noAnsi;
extern int stop;
extern int refreshRate;
extern int gameDelay;
extern int numDecPoints;
extern int timer;
extern int nCpus;
extern int noLibraries;
extern char *bgColor;
extern time_t startTime;
extern int freezeGames;

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
extern int usage(int argc, char *argv[]);
extern void IntOutputLoop();

// Global Thread Table
extern GameThread *gameThreadTable[MAXCORES]; 

