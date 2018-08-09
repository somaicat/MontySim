FLAGS = -lpthread -lncurses

all: monty

debug: FLAGS += -g
debug: monty

release: FLAGS += -Wall -O3
release: monty

monty: monty.o
	gcc monty.o $(FLAGS) -o ./monty

monty.o: monty.c
	gcc -c ./monty.c $(FLAGS) -o monty.o

clean:
	rm ./*.o ./monty
