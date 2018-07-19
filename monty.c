#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/sysinfo.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define MAXCORES 64 // Yeah 64 is probably too many (I think now... till iunno 30 years from now I look back and think "64? a modern cpu is measured in kilocores wtf is this?".

const char *ContestantChoiceStr[] = {"to stick with door", "to change to door"};
int killtime=0;
int verbose=0;
int stop=0;
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

  for (num=0; !killtime;num++) {
    if (verbose) printf(ZEROCURSOR);
    decision = rand_r(&score->seed)%2;
    correctDoor = rand_r(&score->seed) % 3;
    score->percentWonWSwitch = ((double)score->numWonWSwitch / ((double)score->numWonWSwitch+(double)score->numLostWSwitch)) * 100.0f;
    score->percentWonWoSwitch = ((double)score->numWonWoSwitch / ((double)score->numWonWoSwitch+(double)score->numLostWoSwitch)) * 100.0f;
    chosenDoor = rand_r(&score->seed) % 3;

    if (verbose) {
      printf("Score: (%d:%d Wins to loss \\w switch, %d:%d wins to loss \\wo switch)\n", score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
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
      if (verbose) printf(" - Contestant Won \n");
    }
    else {
      if (decision) score->numLostWSwitch++;
      else score->numLostWoSwitch++;
      if (verbose) printf(" - Contestant Lost\n");
    }
      if (stop && verbose)
        getchar(); // Need to make this a command line argument, amount various other things i need to do
    }
}

GameThread *StartGame() {
  int num;
  GameThread *game = calloc(1,sizeof(GameThread));
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
  while(!killtime) {
      printf(ZEROCURSOR);
      for (num=0; gameThreadTable[num] != NULL; num++) {
        score = &gameThreadTable[num]->score;
        printf("[Thread %d] Score: (%llu:%llu Wins to loss \\w switch, %llu:%llu wins to loss \\wo switch)\n",num, score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
        printf("[Thread %d] Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", num, score->percentWonWSwitch, score->percentWonWoSwitch); 
        totalNumWonWSwitch+=score->numWonWSwitch;
        totalNumLostWSwitch+=score->numLostWSwitch;
        totalNumWonWoSwitch+=score->numWonWoSwitch;
        totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      printf("\nTotal percentages %.2f% won switching, %.2f% won without switching\n",totalPercentWonWSwitch, totalPercentWonWoSwitch);
      printf("Total games played: %llu\n",totalGames);
      totalNumWonWSwitch=0;
      totalNumWonWoSwitch=0;
      totalNumLostWSwitch=0;
      totalNumLostWoSwitch=0;
      usleep(300);
  }
}

int main(int argc, int *argv[]) {
  int num;
  pthread_t monThread;
  printf("Installing signal handler...\n");
  signal(SIGINT, sighandler);

  printf(CLEARSCR);

  while ((num = getopt(argc, argv, "sg")) != -1) {
    switch (num) {
      case 's': verbose=1; break;
      case 'g': stop=1; break;
    }
  }

  if (!verbose) {// no verbose, start multithreaded operation
    for (num=0;num<get_nprocs() && num < MAXCORES ;num++) { // Is this future proofing? maybe...
      gameThreadTable[num] = StartGame();
      gameThreadTable[num]->score.seed = (unsigned int) time(NULL);
    }
    MonitorThread();
    for (num=0;num<get_nprocs() && num < MAXCORES ;num++) { // ... or maybe not.
      pthread_join(gameThreadTable[num]->thread, 0);
      free(gameThreadTable[num]);
    }
  }
  else {
    gameThreadTable[0] = calloc(1, sizeof(GameThread));
    PlayGame(&gameThreadTable[0]->score);
    free(gameThreadTable[0]);
  }
  printf("Ending game loop.\n");

};
