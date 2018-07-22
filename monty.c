#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define MAXCORES 64 // Yeah 64 is probably too many (I think now... till iunno 30 years from now I look back and think "64? a modern cpu is measured in kilocores wtf is this?".

// Colors
#define C_G "\e[92m"
#define C_R "\e[91m"
#define C_RST "\e[m"

const char *ContestantChoiceStr[] = {"\e[94mto stick with door\e[m", "\e[93mto change to door\e[m"};
const char *failStr = "Use %s -h for more information\n";
const char *helpStr = \
		"Usage: %s [OPTION]\n"\
		"Monty Hall game simulation\n\n"\
		"  -d\t\tDelay each game by number of ms\n"\
		"  -t\t\tManually set number of threads to use in multithreaded mode\n"\
                "  -r\t\tManually set status refresh rate in ms in multithreaded mode\n"\
		"  -s\t\tSingle threaded verbose mode\n"\
		"  -g\t\tWait for enter at each round when in single threaded mode, no effect otherwise\n"\
		"  -a\t\tNo ANSI terminal controls (no effect in multithreaded mode)\t\t\n"\
		"  -h\t\tDisplays this help informaion\n";

int killtime=0;
int verbose=0;
int noAnsi=0;
int stop=0;
int refreshRate=100;
int gameDelay=0;

typedef struct GameScore { // Might not need volatile here, ill look into it later
  unsigned long long numWonWSwitch;
  unsigned long long numWonWoSwitch;
  unsigned long long numLostWSwitch;
  unsigned long long numLostWoSwitch;
  float percentWonWSwitch;
  float percentWonWoSwitch;
  unsigned int seed; // TODO: yeah this is a stupid place to put this, but i'll fix it later
} GameScore;

typedef struct GameThread {
  pthread_t thread;
  GameScore score;
} GameThread;

GameThread *gameThreadTable[MAXCORES] = {0}; 

void sighandler(int num) {
  killtime=1; // Using a global for now in preparation for future threading support
}

void PlayGame(GameScore *score) {
  int correctDoor;
  int chosenDoor;
  int excludedDoor;
  int decision;
  int altDoor;
  int num;
  struct timespec ts;

  ts.tv_sec = gameDelay/1000;
  ts.tv_nsec = (gameDelay-(gameDelay/1000*1000))*1000000;

  for (num=0; !killtime;num++) {
    if (verbose && !noAnsi) printf(ZEROCURSOR);
    decision = rand_r(&score->seed)%2;
    correctDoor = rand_r(&score->seed) % 3;
    score->percentWonWSwitch = ((double)score->numWonWSwitch / ((double)score->numWonWSwitch+(double)score->numLostWSwitch)) * 100.0f;
    score->percentWonWoSwitch = ((double)score->numWonWoSwitch / ((double)score->numWonWoSwitch+(double)score->numLostWoSwitch)) * 100.0f;
    chosenDoor = rand_r(&score->seed) % 3;

    if (verbose) {
      printf("Score: (%llu:%llu Wins to loss \\w switch, %llu:%llu wins to loss \\wo switch)\n", score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
      printf("Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", score->percentWonWSwitch, score->percentWonWoSwitch); 
      printf(" - Game started, (winning door is %d)\n", correctDoor);
      printf(" - Contestant chose door %d\n", chosenDoor);
    }

    // Pick a door to exclude and alternate to offer
    for (excludedDoor=0;excludedDoor==correctDoor || excludedDoor == chosenDoor;excludedDoor++);
    for (altDoor=0;altDoor == chosenDoor || altDoor == excludedDoor;altDoor++);
    chosenDoor = (decision) ? altDoor:chosenDoor;
    if (verbose) {
      printf(" - Host excluded door %d, option to change to door %d presented\n", excludedDoor, altDoor);
      printf(" - Contestant decided %s %d        \n", ContestantChoiceStr[decision], chosenDoor);
    }
    if (correctDoor == chosenDoor) {
      if (decision) score->numWonWSwitch++;
      else score->numWonWoSwitch++;
      if (verbose) printf(" - %sContestant Won%s      \n", C_G, C_RST);
    }
    else {
      if (decision) score->numLostWSwitch++;
      else score->numLostWoSwitch++;
      if (verbose) printf(" - %sContestant Lost%s     \n", C_R, C_RST);
    }
      if (stop && verbose)
        getchar(); // Need to make this a command line argument, amount various other things i need to do
      if (gameDelay>0) {
        nanosleep(&ts, NULL);
        if (verbose) printf("Delaying by %dms\n", gameDelay);
      }
    }
}

int SeedGame(GameScore *score) {
  score->seed = (unsigned int) random(); // this could probably be better
  return 0;
}

GameThread *StartGame() {
  GameThread *game = calloc(1,sizeof(GameThread));
  SeedGame(&game->score);
  if (verbose)
    PlayGame(&game->score);
  else
    pthread_create(&game->thread, NULL, (void *)(void *) PlayGame,(void*) &game->score);
  return game;
}

void *MonitorThread() {
  float totalPercentWonWSwitch=0;
  float totalPercentWonWoSwitch=0;
  unsigned long long totalNumWonWSwitch=0;
  unsigned long long totalNumWonWoSwitch=0;
  unsigned long long totalNumLostWSwitch=0;
  unsigned long long totalNumLostWoSwitch=0;
  unsigned long long totalGames;
  int num;
  GameScore *score;
  struct timespec ts;

  ts.tv_sec = refreshRate/1000;
  ts.tv_nsec = (refreshRate-(refreshRate/1000*1000))*1000000;

  while(!killtime) {
      printf(ZEROCURSOR);
      for (num=0; gameThreadTable[num] != NULL; num++) {
        score = &gameThreadTable[num]->score;
        printf("[Thread %d] Score: %llu:%llu (%.2f%%) W:L \\w switch, %llu:%llu (%.2f%%) W:L \\wo switch\n",num, score->numWonWSwitch, score->numLostWSwitch, score->percentWonWSwitch, score->numWonWoSwitch, score->numLostWoSwitch, score->percentWonWoSwitch);
//        printf("[Thread %d] Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", num, score->percentWonWSwitch, score->percentWonWoSwitch); 
        totalNumWonWSwitch+=score->numWonWSwitch;
        totalNumLostWSwitch+=score->numLostWSwitch;
        totalNumWonWoSwitch+=score->numWonWoSwitch;
        totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      printf("\nTotal percentages %.2f%% won switching, %.2f%% won without switching\n",totalPercentWonWSwitch, totalPercentWonWoSwitch);
      printf("Total games played: %llu\n",totalGames);
      totalNumWonWSwitch=0;
      totalNumWonWoSwitch=0;
      totalNumLostWSwitch=0;
      totalNumLostWoSwitch=0;
      nanosleep(&ts, NULL); // Sleep main thread for refresh period
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  int num;
  int nCpus = get_nprocs();
  while ((num = getopt(argc, argv, "hsgar:t:d:")) != -1) {
    switch (num) {
      case 's':
        verbose=1;
        break;
      case 'g':
        stop=1;
        break;
      case 'a':
        noAnsi=1;
        break;
      case 'd':
        gameDelay = atoi(optarg);
        if (gameDelay <= 0) {
          printf("%s: Game delay must be above 0ms\n", argv[0]);
          printf(failStr, argv[0]);
          return 0;
        }
        break;
      case 'r':
        refreshRate = atoi(optarg);
        if (refreshRate <= 0) {
          printf("%s: Refresh rate must be above 0ms\n", argv[0]);
          printf(failStr, argv[0]);
          return 0;
        }
        break;
      case 't':
        nCpus = atoi(optarg);
        if (nCpus > 0 && nCpus < MAXCORES) break; // If the argument is retarded, this conditional ensures that we won't break out of the switch and will fall through to the default and return
        printf("%s: Thread number must be between 1 and %d\n", argv[0], MAXCORES); // Retard argument detected, falling through to return
        printf(failStr, argv[0]);
        return 0;
      default:
        printf(failStr, argv[0]);
        return 0;
      case 'h':
        printf(helpStr, argv[0]);
        return 0;
    }
  }

  printf("Installing signal handler...\n");
  signal(SIGINT, sighandler);

  if (!verbose || !noAnsi) printf(CLEARSCR);

  if (!verbose) {// no verbose, start multithreaded operation
    for (num=0;num<nCpus && num < MAXCORES ;num++) { // Is this future proofing? maybe...
      gameThreadTable[num] = StartGame();
    }
    MonitorThread();
    for (num=0;num<nCpus && num < MAXCORES ;num++) { // ... or maybe not.
      pthread_join(gameThreadTable[num]->thread, 0);
      free(gameThreadTable[num]); // Now that we've rejoined all the worker threads, free the memory.
    }
  }
  else {
    gameThreadTable[0] = StartGame();
    free(gameThreadTable[0]);
  }
  printf("Ending game loop.\n");
  return 0;
};
