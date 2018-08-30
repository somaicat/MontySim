#include "monty.h"
#include "usage.h"

int usage(int argc, char *argv[]) {
  int num;
  char *localeStr = NULL;

  while ((num = getopt(argc, argv, OPTS)) != -1) {
    switch (num) {
      case 's':
        verbose=1;
        break;
      case 'g':
        stop=1;
        break;
      case 'n':
        noLibraries=1;
        break;
      case 'S':
        localeStr = setlocale(LC_NUMERIC, "");
        break;
      case 'T':
        if (atoi(optarg) <= 0) {
          ERRORRET(argv[0], "Time limit must be 1 second or more");
        }
        timer=atoi(optarg);
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
      case 'l':
        if (strlen(optarg) >= MAXFILELEN) {
          ERRORRET(argv[0], "Session file length too long");
        }
        strcpy(saveSessionFile, optarg);
        break;
      case 'd':
        gameDelay = atoi(optarg);
        if (gameDelay <= 0) {
          ERRORRET(argv[0], "Game delay must be above 0ms");
        }
        break;
/*      case 'r':
        refreshRate = atoi(optarg);
        if (refreshRate <= 0) {
          ERRORRET(argv[0], "Refresh rate must be above 0ms");
        }
        break;*/
      case 't':
        nCpus = atoi(optarg);
        if (nCpus > 0 && nCpus <= MAXCORES) break; // If the argument is retarded, this conditional ensures that we won't break out of the switch and will fall through to the default and return
        fprintf(stderr, "%s: Thread number must be between 1 and %d\n", argv[0], MAXCORES); // Retard argument detected, falling through to return
        ERRORSTR(argv[0]);
        return 1;
      default:
        ERRORSTR(argv[0]);
        return 1;
      case 'h':
        fprintf(stderr, helpStr, argv[0]);
        return 1;
    }
  }
  if (localeStr) printf("Setting locale to %s\n", localeStr);
  return 0;
};
