FLAGS = -lpthread -lncurses
DEFINES = -DNCURSES

all: monty

debug: FLAGS += -g
debug: monty

release: FLAGS += -Wall -O3
release: monty

noncurses: DEFINES =
noncurses: monty

monty: monty.o ncurses.o
	gcc monty.o ncurses.o $(FLAGS) -o ./monty

monty.o: monty.c
	gcc -c ./monty.c $(FLAGS) $(DEFINES) -o monty.o

ncurses.o: ncurses.c
	gcc -c ./ncurses.c $(FLAGS) $(DEFINES) -o ncurses.o

clean:
	rm ./*.o ./monty
