FLAGS = -lpthread -lncurses
DEFINES = -DNCURSES
FILES = monty.o ncurses.o

all: monty

debug: FLAGS += -g
debug: monty

release: FLAGS += -Wall -O3
release: monty

noncurses: DEFINES =
noncurses: FLAGS = -lpthread
noncurses: FILES = monty.o
noncurses: monty

monty: $(FILES)
	gcc $(FILES) $(FLAGS) -o ./monty

monty.o: monty.c
	gcc -c ./monty.c $(FLAGS) $(DEFINES) -o monty.o

ncurses.o: ncurses.c
	gcc -c ./ncurses.c $(FLAGS) $(DEFINES) -o ncurses.o

clean:
	rm ./*.o ./monty
