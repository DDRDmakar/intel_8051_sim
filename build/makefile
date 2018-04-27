# intel 8051 simulator makefile

CC=gcc
DIR=../code/
OPT := -lm -Os -std=c11
BIN_NAME=sim8051

prog: main.o
	$(CC) $(OPT) -o $(BIN_NAME) main.o

main.o: $(DIR)main.c $(DIR)headers/memory.h
	$(CC) $(OPT) -c $(DIR)main.c

clean:
	rm *.o
