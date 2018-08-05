#include "monty.h"
char *bgColor = C_DOSBG;
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

  GameScore *score = &game->score;

  ts.tv_sec = gameDelay/1000;
  ts.tv_nsec = (gameDelay-(gameDelay/1000*1000))*1000000; // This works by first taking the delay, say.. 1500, it divides and multiplies by 1000, which since C rounds down and this is an integer, just drops it to the nearest 1000. Then it uses that to subtract from the original number  (1500-1000) to get 500, and then multiplies to get nanoseconds, probably a better way to do this.

  for (num=0; !killtime;num++) {
    if (verbose && !noAnsi) printf(ZEROCURSOR);
    decision = rand_r(&game->seed) / (RAND_MAX / 2+1); 
    correctDoor = rand_r(&game->seed) / (RAND_MAX / 3+1);

    score->percentWonWSwitch = ((double)score->numWonWSwitch / ((double)score->numWonWSwitch+(double)score->numLostWSwitch)) * 100.0f;
    score->percentWonWoSwitch = ((double)score->numWonWoSwitch / ((double)score->numWonWoSwitch+(double)score->numLostWoSwitch)) * 100.0f;
    chosenDoor = rand_r(&game->seed) / (RAND_MAX / 3+1);


    if (verbose) {
      printf("Score: (%'llu:%'llu Wins to loss \\w switch, %'llu:%'llu wins to loss \\wo switch)\n", score->numWonWSwitch, score->numLostWSwitch, score->numWonWoSwitch, score->numLostWoSwitch);
      printf("Winning percentage: %.*f%% \\w switch, %.*f%% \\wo switch\n", numDecPoints, score->percentWonWSwitch, numDecPoints, score->percentWonWoSwitch); 
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
  time_t startTime = time(NULL);
  int secondsPast;
  int rt_Hours, rt_Minutes, rt_Seconds;
  ts.tv_sec = refreshRate/1000;
  ts.tv_nsec = (refreshRate-(refreshRate/1000*1000))*1000000;
  int stopLoop=0;

  while(!stopLoop) {
      if (killtime && bgColor == C_DOSBG) printf("%s%s",(bgColor=C_RST),CLEARSCR);
      printf(ZEROCURSOR);
      secondsPast = time(NULL) - startTime;
      rt_Hours = secondsPast / 3600;
      rt_Minutes = (secondsPast % 3600) / 60;
      rt_Seconds = secondsPast % 60;
      printf("%s------ [%d:%d:%d] -----%s\n", (stopLoop = killtime) ? C_R : C_G, rt_Hours, rt_Minutes, rt_Seconds, bgColor); // NOTE: Since the use of the ternary conditional here could very easily be misunderstood, please note that it is CORRECT that there is only one '=' symbol, not two. It is intended that it FIRST set killtime to stopLoop, THEN evaluating it. This is NOT a typo.
      for (num=0; num < MAXCORES && gameThreadTable[num] != NULL; num++) {
        score = &gameThreadTable[num]->score;
        printf("[Thread %d]\n", num+1);
        printf("%s", CLEARLINE);
        printf("Switching:\tWins %'llu\tLoses %'llu\t(%.*f%%)\n", score->numWonWSwitch, score->numLostWSwitch, numDecPoints, score->percentWonWSwitch);
        printf("%s", CLEARLINE);
        printf("Not Switching:\tWins %'llu\tLoses %'llu\t(%.*f%%)\n", score->numWonWoSwitch, score->numLostWoSwitch, numDecPoints, score->percentWonWoSwitch);
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
  return NULL;
}

int main(int argc, char *argv[]) {
  int num;
  int nCpus = get_nprocs();
  struct sigaction sigAct;
  char *localeStr = NULL;
  while ((num = getopt(argc, argv, "hsSgaAp:r:t:d:T:")) != -1) {
    switch (num) {
      case 's':
        verbose=1;
        break;
      case 'g':
        stop=1;
        break;
      case 'S':
        localeStr = setlocale(LC_NUMERIC, "");
        break;
      case 'T':
        if (atoi(optarg) <= 0) {
          ERRORRET(argv[0], "Time limit must be 1 second or more");
        }
        alarm(atoi(optarg));
        break;
      case 'p':
        numDecPoints = atoi(optarg);
        if (numDecPoints > 10 || numDecPoints < 2) {
          ERRORRET(argv[0], "Decimal points must be between 2 and 10");
        }
        break;
      case 'a':
        noAnsi=1;
        break;
      case 'A':
        bgColor = C_RST; 
        break;
      case 'd':
        gameDelay = atoi(optarg);
        if (gameDelay <= 0) {
          ERRORRET(argv[0], "Game delay must be above 0ms");
        }
        break;
      case 'r':
        refreshRate = atoi(optarg);
        if (refreshRate <= 0) {
          ERRORRET(argv[0], "Refresh rate must be above 0ms");
        }
        break;
      case 't':
        nCpus = atoi(optarg);
        if (nCpus > 0 && nCpus <= MAXCORES) break; // If the argument is retarded, this conditional ensures that we won't break out of the switch and will fall through to the default and return
        printf("%s: Thread number must be between 1 and %d\n", argv[0], MAXCORES); // Retard argument detected, falling through to return
        ERRORSTR(argv[0]);
        return 0;
      default:
        ERRORSTR(argv[0]);
        return 0;
      case 'h':
        printf(helpStr, argv[0]);
        return 0;
    }
  }

  memset(&sigAct, 0, sizeof(struct sigaction));
  sigAct.sa_handler = sighandler;
  printf("Installing signal handler...\n");
  sigaction(SIGINT, &sigAct, NULL);
  sigaction(SIGALRM, &sigAct, NULL);
  if (localeStr) printf("Setting locale to %s\n", localeStr);

  if (!verbose) {// no verbose, start multithreaded operation
    printf("%s%s", bgColor, CLEARSCR);
    for (num=0;num<nCpus && num < MAXCORES ;num++) { // Is this future proofing? maybe...
      gameThreadTable[num] = StartGame();
    }
    MonitorThread();
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
  return 0;
};
