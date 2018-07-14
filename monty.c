#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"

typedef struct GameScore {
  int numWonWSwitch;
  int numWonWoSwitch;
  int numLostWSwitch;
  int numLostWoSwitch;

  float percentWonWSwitch;
  float percentWonWoSwitch;
} GameScore;

GameScore *gameScoreTable[64] = {0}; 

const char *ContestantChoiceStr[] = {"to stick with door", "to change to door"};
int killtime=0;

void sighandler(int num) {
  killtime=1; // Using a global for now in preparation for future threading support
}

int main() {
  gameScoreTable[0] = calloc(1, sizeof(GameScore));
//  int numWonWSwitch=0;
//  int numWonWoSwitch=0;
//  int numLostWSwitch=0;
//  int numLostWoSwitch=0;

//  float percentWonWSwitch=0.0f;
//  float percentWonWoSwitch=0.0f;

  long int correctDoor;
  long int chosenDoor;
  long int excludedDoor;
  long int decision;
  long int altDoor;

  int num;
  printf("Installing signal handler...\n");
  signal(SIGINT, sighandler);

  printf(CLEARSCR);
  for (num=0; !killtime;num++) {
    printf(ZEROCURSOR);
    decision = random()%2;
    correctDoor = random() % 3;
    gameScoreTable[0]->percentWonWSwitch = ((float)gameScoreTable[0]->numWonWSwitch / ((float)gameScoreTable[0]->numWonWSwitch+gameScoreTable[0]->numLostWSwitch)) * 100.0f;
    gameScoreTable[0]->percentWonWoSwitch = ((float)gameScoreTable[0]->numWonWoSwitch / ((float)gameScoreTable[0]->numWonWoSwitch+gameScoreTable[0]->numLostWoSwitch)) * 100.0f;
    printf("Score: (%d:%d Wins to loss \\w switch, %d:%d wins to loss \\wo switch)\n", gameScoreTable[0]->numWonWSwitch, gameScoreTable[0]->numLostWSwitch, gameScoreTable[0]->numWonWoSwitch, gameScoreTable[0]->numLostWoSwitch);
    printf("Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", gameScoreTable[0]->percentWonWSwitch, gameScoreTable[0]->percentWonWoSwitch); 
    printf(" - Game %d started, (winning door is %d)\n", num, correctDoor);
    chosenDoor = random() % 3;
    printf(" - Contestant chose door %d\n", chosenDoor);

    // Pick a door to exclude
    for (excludedDoor=0;excludedDoor==correctDoor || excludedDoor == chosenDoor;excludedDoor++);

    // Pick an alternate door to offer
    for (altDoor=0;altDoor == chosenDoor || altDoor == excludedDoor;altDoor++);

    printf(" - Host excluded door %d, option to change to door %d presented\n", excludedDoor, altDoor);
    printf(" - Contestant decided %s %d        \n", ContestantChoiceStr[decision], (chosenDoor = (decision) ? altDoor:chosenDoor));

    if (correctDoor == chosenDoor) {
      if (decision) gameScoreTable[0]->numWonWSwitch++;
      else gameScoreTable[0]->numWonWoSwitch++;
      printf(" - Contestant Won \n");
    }
    else {
      if (decision) gameScoreTable[0]->numLostWSwitch++;
      else gameScoreTable[0]->numLostWoSwitch++;
      printf(" - Contestant Lost\n");
    }

//getchar(); // Need to make this a command line argument, amount various other things i need to do
  }
  free(gameScoreTable[0]);
  printf("Ending game loop.\n");

};
