#define ERRORSTR(s) fprintf(stderr, failStr, s)
#define ERRORRET(t, s) fprintf(stderr, "%s: %s\n", t, s); ERRORSTR(t); return 1

#define OPTS "nhsSgap:t:d:T:l:"

// Constant strings
const static char *failStr = "Use %s -h for more information\n";
const static char *helpStr = \
		"Usage: %s [OPTION]\n"\
		"Monty Hall game simulation\n\n"\
		"  -p <points>\tSpecify number of decimal points displayed (between 2 and 10)\n"\
		"  -d <delay>\tDelay each game by number of ms\n"\
		"  -S\t\tShow thousands seperators\n"\
		"  -h\t\tDisplays this help informaion\n"\
		"\n--------- Multithreaded options\n"\
		"  -t <threads>\tManually set number of threads to use\n"\
		"  -T <timeout>\tStop games after number of seconds\n"\
		"  -n\t\tDon't load any external UI libraries\n"\
		"  -l <file>\tLoad session file\n"\
		"\n--------- Singlethreaded options\n"\
		"  -s\t\tSingle threaded (verbose) mode\n"\
		"  -g\t\tWait for enter at the end of each game\n"\
		"  -a\t\tNo ANSI cursor controls, show ALL games\n";
