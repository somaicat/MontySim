FLAGS = -lpthread -ldl
FILES = monty.o

all: monty ncurses.so

debug: FLAGS += -g
debug: monty ncurses.so

release: FLAGS += -Wall -O3
release: monty ncurses.so

monty: $(FILES)
	gcc -rdynamic $(FILES) $(FLAGS) -o ./monty

monty.o: monty.c
	gcc -c ./monty.c $(FLAGS) -o monty.o

ncurses.so: ncurses.c
	gcc -fPIC -c ./ncurses.c 
	gcc -shared -o ./ncurses.so ./ncurses.o -lncurses

clean:
	rm ./*.o ./monty ./*.so
