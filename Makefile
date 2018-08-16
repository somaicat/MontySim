LIBS = -lpthread -ldl
FILES = monty.o usage.o
OUTPUT = monty ncurses.so

all: release

debug: FLAGS = -ggdb
debug: $(OUTPUT)

release: FLAGS = -Wall -O3
release: $(OUTPUT)
	@strip $(OUTPUT)

monty: $(FILES)
	gcc -rdynamic $(FILES) $(LIBS) $(FLAGS) -o ./monty

usage.o: usage.c
	gcc -c ./usage.c $(LIBS) $(FLAGS) -o usage.o

monty.o: monty.c
	gcc -c ./monty.c $(LIBS) $(FLAGS) -o monty.o

ncurses.so: ncurses.c
	gcc $(FLAGS) -fPIC -c ./ncurses.c 
	gcc $(FLAGS) -shared -o ./ncurses.so ./ncurses.o -lncurses

clean:
	@echo "Deleting object files"
	@rm ./*.o ./monty ./*.so
