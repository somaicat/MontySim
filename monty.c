#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define CPUNUM 4 // placeholder till I have detection coded
const char *ContestantChoiceStr[] = {"to stick with door", "to change to door"};
int killtime=0;
int verbose=0;
int stop=0;
typedef struct GameScore { // Might not need volatile here, ill look into it later
  volatile int numWonWSwitch;
  volatile int numWonWoSwitch;
  volatile int numLostWSwitch;
  volatile int numLostWoSwitch;

  volatile float percentWonWSwitch;
  volatile float percentWonWoSwitch;
  unsigned int seed; // TODO: yeah this is a stupid place to put this, but i'll fix it later
} GameScore;

typedef struct GameThread {
  pthread_t thread;
  GameScore score;
} GameThread;

GameThread *gameThreadTable[64] = {0}; 


void sighandler(int num) {
  killtime=1; // Using a global for now in preparation for future threading support
}

void PlayGame(GameScore *score) {
  long int correctDoor;
  long int chosenDoor;
  long int excludedDoor;
  long int decision;
  long int altDoor;
  int num;

  for (num=0; !killtime;num++) {
    if (verbose) printf(ZEROCURSOR);
    decision = rand_r(&score->seed)%2;
    correctDoor = rand_r(&score->seed) % 3;
    score->percentWonWSwitch = ((float)score->numWonWSwitch / ((float)score->numWonWSwitch+score->numLostWSwitch)) * 100.0f;
    score->percentWonWoSwitch = ((float)score->numWonWoSwitch / ((float)score->numWonWoSwitch+score->numLostWoSwitch)) * 100.0f;
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

void *MonitorThread(void *arg) {
  float totalPercentWonWSwitch=0;
  float totalPercentWonWoSwitch=0;
  unsigned int totalNumWonWSwitch=0;
  unsigned int totalNumWonWoSwitch=0;
  unsigned int totalNumLostWSwitch=0;
  unsigned int totalNumLostWoSwitch=0;
  unsigned int totalGames;
  int num;
  GameScore *score;
  while(!killtime) {
      printf(ZEROCURSOR);
      for (num=0; gameThreadTable[num] != NULL; num++) {
        score = &gameThreadTable[num]->score;
        printf("[Thread %d] Score: (%d:%d Wins to loss \\w switch, %d:%d wins to loss \\wo switch)\n",num, score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
        printf("[Thread %d] Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", num, score->percentWonWSwitch, score->percentWonWoSwitch); 
        totalNumWonWSwitch+=score->numWonWSwitch;
        totalNumLostWSwitch+=score->numLostWSwitch;
        totalNumWonWoSwitch+=score->numWonWoSwitch;
        totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      totalPercentWonWSwitch = ((float)totalNumWonWSwitch / ((float)totalNumWonWSwitch+totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((float)totalNumWonWoSwitch / ((float)totalNumWonWoSwitch+totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      printf("\nTotal percentages %.2f% won switching, %.2f% won without switching\n",totalPercentWonWSwitch, totalPercentWonWoSwitch);
      printf("Total games played: %u\n",totalGames);
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
    for (num=0;num<CPUNUM;num++) { 
      gameThreadTable[num] = StartGame();
      gameThreadTable[num]->score.seed = (unsigned int) time(NULL);
    }
    pthread_create(&monThread, NULL, (void *)(void *) MonitorThread, NULL);
    for (num=0;num<CPUNUM;num++) { 
      pthread_join(gameThreadTable[num]->thread, 0);
      free(gameThreadTable[num]);
    }
    pthread_join(monThread, 0);
  }
  else {
    gameThreadTable[0] = calloc(1, sizeof(GameThread));
    PlayGame(&gameThreadTable[0]->score);
    free(gameThreadTable[0]);
  }
  printf("Ending game loop.\n");

};
