#include "monty.h"

// Globals
int killtime=0;
int verbose=0;
int noAnsi=0;
int stop=0;
int refreshRate=100;
int gameDelay=0;
int numDecPoints=2;
int timer=0;
int nCpus=1;
int noLibraries=0;
int freezeGames=0;

char *bgColor = C_RST;
GameThread *gameThreadTable[MAXCORES] = {0};
void (*ExtOutputLoop)() = NULL;
time_t startTime = 0;

void sighandler(int sig) {
  killtime=sig; // Using a global for now in preparation for future threading support
}

void PlayGame(GameThread *game) {
  int correctDoor;
  int chosenDoor;
  int excludedDoor;
  int decision;
  int altDoor;
  int num;
  struct timespec ts;
  const char *ContestantChoiceStr[] = {"\e[94mto stick with door\e[m", "\e[93mto change to door\e[m"};

  GameScore *score = &game->score;

  ts.tv_sec = gameDelay/1000;
  ts.tv_nsec = (gameDelay-(gameDelay/1000*1000))*1000000; // This works by first taking the delay, say.. 1500, it divides and multiplies by 1000, which since C rounds down and this is an integer, just drops it to the nearest 1000. Then it uses that to subtract from the original number  (1500-1000) to get 500, and then multiplies to get nanoseconds, probably a better way to do this.

  for (num=0; !killtime;num++) {
    if (freezeGames) {
      nanosleep(&ts, NULL);
      continue;
    }
    if (verbose && !noAnsi) printf(ZEROCURSOR);
    decision = rand_r(&game->seed) / (RAND_MAX / 2+1); 
    correctDoor = rand_r(&game->seed) / (RAND_MAX / 3+1);

    score->percentWonWSwitch = ((double)score->numWonWSwitch / ((double)score->numWonWSwitch+(double)score->numLostWSwitch)) * 100.0f;
    score->percentWonWoSwitch = ((double)score->numWonWoSwitch / ((double)score->numWonWoSwitch+(double)score->numLostWoSwitch)) * 100.0f;
    chosenDoor = rand_r(&game->seed) / (RAND_MAX / 3+1);


    if (verbose) {
      printf("Score: (%'llu:%'llu Wins to loss \\w switch, %'llu:%'llu wins to loss \\wo switch)\n", score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
      printf("Winning percentage: %.*f%% \\w switch, %.*f%% \\wo switch\n", numDecPoints, score->percentWonWSwitch, numDecPoints, score->percentWonWoSwitch); 
      printf("Total games: %'llu\n", score->numWonWSwitch+score->numWonWoSwitch+score->numLostWSwitch+score->numLostWoSwitch);
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
      if (verbose) printf(" - %sContestant Won%s      \n", C_G, bgColor);
    }
    else {
      if (decision) score->numLostWSwitch++;
      else score->numLostWoSwitch++;
      if (verbose) printf(" - %sContestant Lost%s     \n", C_R, bgColor);
    }
      if (stop && verbose)
        getchar(); // Need to make this a command line argument, amount various other things i need to do
      if (gameDelay>0) {
        if (verbose) printf("Delaying by %dms\n", gameDelay);
        nanosleep(&ts, NULL);
      }
    }
}

int SeedGame(GameThread *game) {
  game->seed = (unsigned int) random(); // this could probably be better
  return 0;
}

GameThread *StartGame() {
  GameThread *game = calloc(1,sizeof(GameThread));
  SeedGame(game);
  if (verbose)
    PlayGame(game);
  else
    pthread_create(&game->thread, NULL, (void *)(void *) PlayGame,(void*) game);
  return game;
}

void IntOutputLoop() {
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
  if (startTime == 0) startTime = time(NULL);
  int secondsPast;
  int rt_Hours, rt_Minutes, rt_Seconds;
  ts.tv_sec = refreshRate/1000;
  ts.tv_nsec = (refreshRate-(refreshRate/1000*1000))*1000000;
  int stopLoop=0;

  while(!stopLoop) {
      if (killtime && !strcmp(bgColor,C_DOSBG)) printf("%s%s",(bgColor=C_RST),CLEARSCR);
      secondsPast = time(NULL) - startTime;
      if (timer > 0 && secondsPast >= timer) killtime = SIGALRM;
      printf(ZEROCURSOR);
      rt_Hours = secondsPast / 3600;
      rt_Minutes = (secondsPast % 3600) / 60;
      rt_Seconds = secondsPast % 60;
      printf("%s------ [%d:%d:%d] -----%s\n", (stopLoop = killtime) ? C_R : C_G, rt_Hours, rt_Minutes, rt_Seconds, bgColor); // NOTE: Since the use of the ternary conditional here could very easily be misunderstood, please note that it is CORRECT that there is only one '=' symbol, not two. It is intended that it FIRST set killtime to stopLoop, THEN evaluating it. This is NOT a typo.
      for (num=0; num < MAXCORES && gameThreadTable[num] != NULL; num++) {
        score = &gameThreadTable[num]->score;
        printf("[Thread %d]\n", num+1);
        printf("%s", CLEARLINE);
        printf("Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWSwitch, score->numLostWSwitch, numDecPoints, score->percentWonWSwitch);
        printf("%s", CLEARLINE);
        printf("Not Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWoSwitch, score->numLostWoSwitch, numDecPoints, score->percentWonWoSwitch);
        totalNumWonWSwitch+=score->numWonWSwitch;
        totalNumLostWSwitch+=score->numLostWSwitch;
        totalNumWonWoSwitch+=score->numWonWoSwitch;
        totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      printf("\nTotal percentages %.*f%% won switching, %.*f%% won without switching\n", numDecPoints,totalPercentWonWSwitch, numDecPoints, totalPercentWonWoSwitch);
      printf("Total games played: %'llu\n",totalGames);
      totalNumWonWSwitch=0;
      totalNumWonWoSwitch=0;
      totalNumLostWSwitch=0;
      totalNumLostWoSwitch=0;
      nanosleep(&ts, NULL); // Sleep main thread for refresh period
  }
  return;
}

int main(int argc, char *argv[]) {
  int num;
  struct sigaction sigAct;
  void *extLibrary=NULL;
  nCpus=get_nprocs();

  if (usage(argc,argv)) return 1;
  if (!noLibraries) {
    if ((extLibrary = dlopen("./uioutput.so", RTLD_NOW)) != NULL && (ExtOutputLoop = dlsym(extLibrary, "ExtOutputLoop")) != NULL) {
      printf("Found external output library\n");
    }
    else if ((extLibrary = dlopen("./ncurses.so", RTLD_NOW)) == NULL)
      printf("%s\nncurses support not found\n",dlerror());
    else if ((ExtOutputLoop = dlsym(extLibrary, "ExtOutputLoop")) == NULL) {
      printf("ncurses library corrupt, defaulting to internal UI\n");
      getchar();
    }
    else
      printf("Found ncurses library\n");
  }
  memset(&sigAct, 0, sizeof(struct sigaction));
  sigAct.sa_handler = sighandler;
  printf("Installing signal handler...\n");
  sigaction(SIGINT, &sigAct, NULL);
  sigaction(SIGALRM, &sigAct, NULL);

  if (!verbose) {// no verbose, start multithreaded operation
    if (ExtOutputLoop == NULL) {
      printf("%s%s", bgColor, CLEARSCR);
    }
    for (num=0;num<nCpus && num < MAXCORES ;num++) { // Is this future proofing? maybe...
      gameThreadTable[num] = StartGame();
    }
    if (ExtOutputLoop == NULL)
      IntOutputLoop();
    else
      ExtOutputLoop();
    for (num=0;num<nCpus && num < MAXCORES ;num++) { // ... or maybe not. 
      pthread_join(gameThreadTable[num]->thread, 0);
      free(gameThreadTable[num]); // Now that we've rejoined all the worker threads, free the memory.
    }
  }
  else { // We're gonna run single threaded (slow) mode
    bgColor = C_RST;
    if (!noAnsi) printf(CLEARSCR);
    gameThreadTable[0] = StartGame();
    free(gameThreadTable[0]);
  }
  if (killtime == SIGALRM) printf("Timeout was reached\n");
  printf("%sEnding game loop.\n", SETCURSORLEFT);
  if (extLibrary != NULL) dlclose(extLibrary);
  return 0;
};
