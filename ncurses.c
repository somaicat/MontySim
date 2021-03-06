#include "monty.h"
#include <ncurses.h>

struct NCursesWindows {
WINDOW *threadWin;
WINDOW *totalWin;
WINDOW *titleWin;
WINDOW *win;
} NCursesWindows;

struct NCursesWindows nwins={0};

void InitNCurses() {
  curs_set(0);					// Turn off cursor
  start_color();				// Turn on colors NOTE: add conditional in case colors not supported
  init_pair(1, COLOR_GREEN, COLOR_BLACK);	// Add white on blue color pair
  init_pair(2, COLOR_WHITE, COLOR_CYAN);		// Add white on red color pair
  init_pair(3, COLOR_WHITE, COLOR_BLACK);	// Add white on blue color pair
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);		// Important Text Color
//  attron(COLOR_PAIR(1));			// Turn on WoB color pair
}

void BuildWindows() {
  int y, x;
  getmaxyx(nwins.win, y, x);				// Get max window size
  nwins.titleWin = newwin(1,x,0,0);			// Create title window
  nwins.threadWin = subwin(nwins.win,y-5, x, 5, 0);		// Create thread info window
  nwins.totalWin = subwin(nwins.win,4, x, 1, 0);		// Create totals Window
  wbkgd(nwins.titleWin, COLOR_PAIR(2)|A_BOLD);	// Activate title window colors
  wbkgd(nwins.win, COLOR_PAIR(1)|A_BOLD);		// Activate main window colors
  wbkgd(nwins.threadWin, COLOR_PAIR(3)|A_BOLD);	// Activate thread window colors
  scrollok(nwins.threadWin, TRUE);			// Turn on thread window scrolling
  wbkgd(nwins.totalWin, COLOR_PAIR(1)|A_BOLD);		// Activate total windows colors
  box(nwins.totalWin, 0,0);				// Draw total windows box

  refresh();
  wrefresh(nwins.threadWin);
  wrefresh(nwins.titleWin);
  wrefresh(nwins.totalWin);
  noecho();
  nodelay(nwins.win, TRUE);
}

void ExtOutputLoop() {
  float totalPercentWonWSwitch=0;
  float totalPercentWonWoSwitch=0;
  unsigned long long totalNumWonWSwitch, totalNumWonWoSwitch, totalNumLostWSwitch, totalNumLostWoSwitch;
  unsigned long long totalGames;
  struct timespec ts;
  int y=0;
  int x=0;
  int secondsPast;
  int num;
  int rt_Hours, rt_Minutes, rt_Seconds;
  startTime = time(NULL);
  ts.tv_sec = refreshRate/1000;
  ts.tv_nsec = (refreshRate-(refreshRate/1000*1000))*1000000;
  GameScore *score;
  char buf[256];
  int offsetx,offsety;
  int key;
  if ((nwins.win = initscr()) == NULL) {
  printf("Initialization of ncurses failed!\n");
  }

  InitNCurses();
  BuildWindows();

  while (!killtime) {
    if ((key = getch()) != ERR) { // A key has been pressed
      switch (key) {
        case 'Q':
        case 'q': killtime=1;break;
        case 'F': 
        case 'f': freezeGames = !freezeGames; break;
      }
    }

    offsety=y;offsetx=x;
    getmaxyx(nwins.win, y, x);
    if (offsety != y || offsetx != x) { // Since x and y start initalized to 0, this will always run at least once
      wresize(nwins.threadWin, y-5,x);
      wclear(nwins.totalWin);
      wclear(nwins.titleWin);
      mvwprintw(nwins.titleWin, 0,0,"Monty Hall Simulations Running");
    }
    totalNumWonWSwitch=0;
    totalNumWonWoSwitch=0;
    totalNumLostWSwitch=0;
    totalNumLostWoSwitch=0;
    secondsPast = time(NULL) - startTime;
    if (timer > 0 && secondsPast >= timer) killtime = SIGALRM;
    rt_Hours = secondsPast / 3600;
    rt_Minutes = (secondsPast % 3600) / 60;
    rt_Seconds = secondsPast % 60;
    sprintf(buf, "%d:%.2d:%.2d\n", rt_Hours, rt_Minutes, rt_Seconds);

    mvwprintw(nwins.titleWin, 0,x-strlen(buf),"%s", buf);

    wclear(nwins.threadWin);
    wmove(nwins.threadWin, 2,0);
    for (num=0; num < MAXCORES && gameThreadTable[num] != NULL; num++) {
      score = &gameThreadTable[num]->score;
      wprintw(nwins.threadWin, "  [Thread %d]\n", num+1);
      wprintw(nwins.threadWin, "  Switching:");
      getyx(nwins.threadWin, offsety, offsetx);
      mvwprintw(nwins.threadWin, offsety, x/4, "Wins: %'llu", score->numWonWSwitch);
      mvwprintw(nwins.threadWin, offsety, x/2, "Loses: %'llu", score->numLostWSwitch);
      sprintf(buf, "Odds: %.*f%% \n", numDecPoints, score->percentWonWSwitch);
      mvwprintw(nwins.threadWin, offsety, x-(strlen(buf)), "%s", buf);

      wprintw(nwins.threadWin, "  Non Switching:");
      getyx(nwins.threadWin, offsety, offsetx);
      mvwprintw(nwins.threadWin, offsety, x/4, "Wins: %'llu", score->numWonWoSwitch);
      mvwprintw(nwins.threadWin, offsety, x/2, "Loses: %'llu", score->numLostWoSwitch);
      sprintf(buf, "Odds: %.*f%% \n", numDecPoints, score->percentWonWoSwitch);
      mvwprintw(nwins.threadWin, offsety, x-(strlen(buf)), "%s", buf);

      totalNumWonWSwitch+=score->numWonWSwitch;
      totalNumLostWSwitch+=score->numLostWSwitch;
      totalNumWonWoSwitch+=score->numWonWoSwitch;
      totalNumLostWoSwitch+=score->numLostWoSwitch;
    }
    totalPercentWonWSwitch = ((double)totalNumWonWSwitch / ((double)totalNumWonWSwitch+(double)totalNumLostWSwitch)) * 100.0f;
    totalPercentWonWoSwitch = ((double)totalNumWonWoSwitch / ((double)totalNumWonWoSwitch+(double)totalNumLostWoSwitch)) * 100.0f;
    totalGames = totalNumWonWSwitch + totalNumWonWoSwitch + totalNumLostWSwitch + totalNumLostWoSwitch;

//    mvwprintw(nwins.totalWin, 1,1,"Total Percentages:");
    sprintf(buf, "Won: %'llu Lost: %'llu ", totalNumWonWSwitch, totalNumLostWSwitch);
    mvwprintw(nwins.totalWin, 1,x-strlen(buf)-1, "%s", buf);
    mvwprintw(nwins.totalWin, 1,1," Switching Wins: %.*f%%", numDecPoints, totalPercentWonWSwitch);
    sprintf(buf, "Won: %'llu Lost: %'llu ", totalNumWonWoSwitch, totalNumLostWoSwitch);
    mvwprintw(nwins.totalWin, 2,x-strlen(buf)-1, "%s", buf);
    mvwprintw(nwins.totalWin, 2,1," Non Switching Wins: %.*f%%", numDecPoints, totalPercentWonWoSwitch);
    box(nwins.threadWin, 0,0);
    box(nwins.totalWin, 0,0);
    mvwprintw(nwins.totalWin, 0,2," Totals ");

    wattron(nwins.totalWin, COLOR_PAIR(4)|A_BOLD);			// Turn on temporary colors
    sprintf(buf, " Completed Games - %'llu ", totalGames);
    mvwprintw(nwins.totalWin, 0,x-strlen(buf)-2,"%s",buf);
    wattroff(nwins.totalWin, COLOR_PAIR(4)|A_BOLD);			// Turn off temporary colors

    mvwprintw(nwins.threadWin, 0,2," Thread Outputs ");
    sprintf(buf, " Total Threads - %d ", nCpus);
    mvwprintw(nwins.threadWin, 0,x-strlen(buf)-2,"%s", buf);
    wrefresh(nwins.titleWin);
    wrefresh(nwins.totalWin);
    wrefresh(nwins.threadWin);
    nanosleep(&ts, NULL);
  }
  endwin();
  printf(CLEARSCR);
  IntOutputLoop(); // Call the non-ncurses monitor code. It'll give one listing of the results before finishing since killtime has been reached. Allowing us to see the results after ncurses display stops

  return;
}
