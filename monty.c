#include <stdio.h>
#include <stdlib.h>

#define CLEARSCR "\033[2J"
#define ZEROCURSOR "\033[H"

const char *ContestantChoiceStr[] = {"to stick with door", "to change to door"};

int main() {
  int numWonWSwitch=0;
  int numWonWoSwitch=0;
  int numLostWSwitch=0;
  int numLostWoSwitch=0;

  float percentWonWSwitch=0.0f;
  float percentWonWoSwitch=0.0f;

  long int correctDoor;
  long int chosenDoor;
  long int excludedDoor;
  long int decision;
  long int altDoor;

  int num;

  printf(CLEARSCR);
  for (num=0;;num++) {
    printf(ZEROCURSOR);
    decision = random()%2;
    correctDoor = random() % 3;
    percentWonWSwitch = ((float)numWonWSwitch / ((float)numWonWSwitch+numLostWSwitch)) * 100.0f;
    percentWonWoSwitch = ((float)numWonWoSwitch / ((float)numWonWoSwitch+numLostWoSwitch)) * 100.0f;
    printf("Beginning game %d (%d:%d Wins to loss \\w switch, %d:%d wins to loss \\wo switch)\n", num, numWonWSwitch, numLostWSwitch, numWonWoSwitch, numLostWoSwitch);
    printf("Winning percentage: %.2f%% \\w switch, %.2f%% \\wo switch\n", percentWonWSwitch, percentWonWoSwitch); 
    printf(" - Game started, (winning door is %d)\n", correctDoor);
    chosenDoor = random() % 3;
    printf(" - Contestant chose door %d\n", chosenDoor);

    do // Pick a door to exclude
      excludedDoor = random() % 3;
    while(excludedDoor == correctDoor || excludedDoor == chosenDoor);

    do // Pick an alternate door to offer 
      altDoor = random() % 3;  // NOTE: There might be a better way to do this
    while(altDoor == chosenDoor || altDoor == excludedDoor);

    printf(" - Host excluded door %d, option to change to door %d presented\n", excludedDoor, altDoor);
    printf(" - Contestant decided %s %d        \n", ContestantChoiceStr[decision], (chosenDoor = (decision) ? altDoor:chosenDoor));

    if (correctDoor == chosenDoor) {
      if (decision) numWonWSwitch++;
      else numWonWoSwitch++;
      printf(" - Contestant Won \n");
    }
    else {
      if (decision) numLostWSwitch++;
      else numLostWoSwitch++;
      printf(" - Contestant Lost\n");
    }
//getchar(); // Need to make this a command line argument, amount various other things i need to do
  }
};
