#include "monty.h"
#include <ncurses.h>
struct NCursesWindows {
WINDOW *threadWin;
WINDOW *totalWin;
WINDOW *titleWin;
WINDOW *win;
} NCursesWindows;

struct NCursesWindows nwins;

void InitNCurses() {
  curs_set(0);					// Turn off cursor
  start_color();				// Turn on colors NOTE: add conditional in case colors not supported
  init_pair(1, COLOR_WHITE, COLOR_BLUE);	// Add white on blue color pair
  init_pair(2, COLOR_WHITE, COLOR_RED);		// Add white on red color pair
  attron(COLOR_PAIR(1));			// Turn on WoB color pair
}

void BuildWindows() {
  int y, x;
  getmaxyx(nwins.win, y, x);				// Get max window size
  nwins.titleWin = subwin(nwins.win,1,x,0,0);			// Create title window
  nwins.threadWin = subwin(nwins.win,y-5, x, 1, 0);		// Create thread info window
  wbkgd(nwins.titleWin, COLOR_PAIR(2)|A_BOLD);	// Activate title window colors
  wbkgd(nwins.win, COLOR_PAIR(2)|A_BOLD);		// Activate main window colors
  wbkgd(nwins.threadWin, COLOR_PAIR(1)|A_BOLD);	// Activate thread window colors
  scrollok(nwins.threadWin, TRUE);			// Turn on thread window scrolling
  nwins.totalWin = subwin(nwins.win,4, x, y-4, 0);		// Create totals Window
  wbkgd(nwins.totalWin, COLOR_PAIR(1)|A_BOLD);		// Activate total windows colors
  box(nwins.totalWin, 0,0);				// Draw total windows box
  refresh();
  wrefresh(nwins.threadWin);
  wrefresh(nwins.totalWin);
}

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

  if ((nwins.win = initscr()) == NULL) {
  printf("Initialization of ncurses failed!\n");
  }

  InitNCurses();
  BuildWindows();
  getmaxyx(nwins.win, y, x);

  // Build NCurses window
  while (!killtime) {
    secondsPast = time(NULL) - startTime;
//      if (timer > 0 && secondsPast >= timer) killtime = SIGALRM;
    rt_Hours = secondsPast / 3600;
    rt_Minutes = (secondsPast % 3600) / 60;
    rt_Seconds = secondsPast % 60;
    sprintf(timerBuf, "%d:%d:%d\n", rt_Hours, rt_Minutes, rt_Seconds);

    mvwprintw(nwins.titleWin, 0,0,"Monty Hall Simulations Running");
    mvwprintw(nwins.titleWin, 0,x-strlen(timerBuf),"%s", timerBuf);
// -------------
      wclear(nwins.threadWin);
    for (num=0; num < MAXCORES && gameThreadTable[num] != NULL; num++) {
      score = &gameThreadTable[num]->score;
      wprintw(nwins.threadWin, "\n [Thread %d]\n", num+1);
      wprintw(nwins.threadWin, " Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWSwitch, score->numLostWSwitch, numDecPoints, score->percentWonWSwitch);
      wprintw(nwins.threadWin, " Not Switching:\tWins %'llu\t\tLoses %'llu\t(%.*f%%)\n", score->numWonWoSwitch, score->numLostWoSwitch, numDecPoints, score->percentWonWoSwitch);

      totalNumWonWSwitch+=score->numWonWSwitch;
      totalNumLostWSwitch+=score->numLostWSwitch;
      totalNumWonWoSwitch+=score->numWonWoSwitch;
      totalNumLostWoSwitch+=score->numLostWoSwitch;
      }
      box(nwins.threadWin, 0,0);
      totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
      totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
      totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;
      mvwprintw(nwins.totalWin, 1,1,"Total percentages %.*f%% won switching, %.*f%% won without switching", numDecPoints,totalPercentWonWSwitch, numDecPoints, totalPercentWonWoSwitch);
      mvwprintw(nwins.totalWin, 2,1,"Total games played: %'llu",totalGames);
      totalNumWonWSwitch=0;
      totalNumWonWoSwitch=0;
      totalNumLostWSwitch=0;
      totalNumLostWoSwitch=0;
// ----------
//    refresh();
    wrefresh(nwins.totalWin);
    wrefresh(nwins.titleWin);
    wrefresh(nwins.threadWin);
    nanosleep(&ts, NULL);
  }
  endwin();
}
