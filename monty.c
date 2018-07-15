#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"
#define CPUNUM 4 // placeholder till I have detection coded
const char *ContestantChoiceStr[] = {"to stick with door", "to change to door"};
int killtime=0;

typedef struct GameScore {
  int numWonWSwitch;
  int numWonWoSwitch;
  int numLostWSwitch;
  int numLostWoSwitch;

  float percentWonWSwitch;
  float percentWonWoSwitch;
} GameScore;

typedef struct GameThread {
  pthread_t *thread;
  GameScore score;
} GameThread;

GameThread *gameThreadTable[64] = {0}; 


void sighandler(int num) {
  killtime=1; // Using a global for now in preparation for future threading support
}

void *StartModel(int numOfThreads) {

}


void PlayRound(GameScore *score, int verbose) {
  long int correctDoor;
  long int chosenDoor;
  long int excludedDoor;
  long int decision;
  long int altDoor;

  decision = random()%2;
  correctDoor = random() % 3;
  score->percentWonWSwitch = ((float)score->numWonWSwitch / ((float)score->numWonWSwitch+score->numLostWSwitch)) * 100.0f;
  score->percentWonWoSwitch = ((float)score->numWonWoSwitch / ((float)score->numWonWoSwitch+score->numLostWoSwitch)) * 100.0f;
  chosenDoor = random() % 3;

  if (verbose) {
    printf("Score: (%d:%d Wins to loss \\w switch, %d:%d wins to loss \\wo switch)\n", score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
    printf("Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", score->percentWonWSwitch, score->percentWonWoSwitch); 
    printf(" - Game started, (winning door is %d)\n", correctDoor);
    printf(" - Contestant chose door %d\n", chosenDoor);
  }

  // Pick a door to exclude and alternate to offer
  for (excludedDoor=0;excludedDoor==correctDoor || excludedDoor == chosenDoor;excludedDoor++);
  for (altDoor=0;altDoor == chosenDoor || altDoor == excludedDoor;altDoor++);
  if (verbose) {
    printf(" - Host excluded door %d, option to change to door %d presented\n", excludedDoor, altDoor);
    printf(" - Contestant decided %s %d        \n", ContestantChoiceStr[decision], (chosenDoor = (decision) ? altDoor:chosenDoor));
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
}

int main(int argc, int *argv[]) {
  gameThreadTable[0] = calloc(1, sizeof(GameThread));
  int singleThreadMode=0;
  int num;
  printf("Installing signal handler...\n");
  signal(SIGINT, sighandler);

  if (argc > 1 && !strcasecmp(argv[1], "-s"))
    singleThreadMode = 1;
  else {
    for (num=0;num<CPUNUM;num++) { 

    }
  }

  printf(CLEARSCR);
  for (num=0; !killtime;num++) {
    printf(ZEROCURSOR);
    PlayRound(&gameThreadTable[0]->score, 1);
    if (singleThreadMode)
      getchar(); // Need to make this a command line argument, amount various other things i need to do
  }
  free(gameThreadTable[0]);
  printf("Ending game loop.\n");

};
