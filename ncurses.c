#include "monty.h"
#include <ncurses.h>

void MonitorNCurses() {
  WINDOW *win, *threadWin, *totalWin, *titleWin;
  float totalPercentWonWSwitch=0;
  float totalPercentWonWoSwitch=0;
  unsigned long long totalNumWonWSwitch=0;
  unsigned long long totalNumWonWoSwitch=0;
  unsigned long long totalNumLostWSwitch=0;
  unsigned long long totalNumLostWoSwitch=0;
  unsigned long long totalGames;

  struct timespec ts;
  int y, x;
  int secondsPast;
  int num;
  int rt_Hours, rt_Minutes, rt_Seconds;
  time_t startTime = time(NULL);
  ts.tv_sec = refreshRate/1000;
  ts.tv_nsec = (refreshRate-(refreshRate/1000*1000))*1000000;
  GameScore *score;
  char timerBuf[128];

  if ((win = initscr()) == NULL) {
  printf("Initialization of ncurses failed!\n");
  }
  curs_set(0);
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  init_pair(2, COLOR_WHITE, COLOR_RED);
  attron(COLOR_PAIR(1));
  getmaxyx(win, y, x);
  titleWin = newwin(1,x,0,0);
  threadWin = newwin(y-5, x, 1, 0);
  wbkgd(titleWin, COLOR_PAIR(2));
  wbkgd(win, COLOR_PAIR(1));
  wbkgd(threadWin, COLOR_PAIR(1));
  scrollok(threadWin, TRUE);
  totalWin = newwin(4, x, y-4, 0);
  wbkgd(totalWin, COLOR_PAIR(1));
  box(totalWin, 0,0);
  refresh();
  wrefresh(threadWin);
  wrefresh(totalWin);
  while (!killtime) {
    secondsPast = time(NULL) - startTime;
//      if (timer > 0 && secondsPast >= timer) killtime = SIGALRM;
    rt_Hours = secondsPast / 3600;
    rt_Minutes = (secondsPast % 3600) / 60;
    rt_Seconds = secondsPast % 60;
    sprintf(timerBuf, "%d:%d:%d\n", rt_Hours, rt_Minutes, rt_Seconds);

    mvwprintw(titleWin, 0,0,"Monty Hall Simulations Running");
    mvwprintw(titleWin, 0,x-strlen(timerBuf),"%s", timerBuf);
// -------------
      wclear(threadWin);
    for (num=0; num < MAXCORES && gameThreadTable[num] != NULL; num++) {
      score = &gameThreadTable[num]->score;
      wprintw(threadWin, "\n [Thread %d]\n", num+1);
      wprintw(threadWin, " Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWSwitch, score->numLostWSwitch, numDecPoints, score->percentWonWSwitch);
      wprintw(threadWin, " Not Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWoSwitch, score->numLostWoSwitch, numDecPoints, score->percentWonWoSwitch);

      totalNumWonWSwitch+=score->numWonWSwitch;
      totalNumLostWSwitch+=score->numLostWSwitch;
      totalNumWonWoSwitch+=score->numWonWoSwitch;
      totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      box(threadWin, 0,0);
      totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      mvwprintw(totalWin, 1,1,"Total percentages %.*f%% won switching, %.*f%% won without switching", numDecPoints,totalPercentWonWSwitch, numDecPoints, totalPercentWonWoSwitch);
      mvwprintw(totalWin, 2,1,"Total games played: %'llu",totalGames);
      totalNumWonWSwitch=0;
      totalNumWonWoSwitch=0;
      totalNumLostWSwitch=0;
      totalNumLostWoSwitch=0;
// ----------
//    refresh();
    wrefresh(totalWin);
    wrefresh(titleWin);
    wrefresh(threadWin);
    nanosleep(&ts, NULL);
  }
  endwin();
}
