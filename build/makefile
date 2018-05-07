# intel 8051 simulator makefile

CC=gcc
DIR=../code/
OPT := -lm -Os -std=c11 -l jansson
BIN_NAME=sim8051

prog: main.o mnemonic.o
	$(CC) $(OPT) -o $(BIN_NAME) main.o mnemonic.o

main.o: $(DIR)main.c $(DIR)headers/memory.h
	$(CC) $(OPT) -c $(DIR)main.c

mnemonic.o: $(DIR)mnemonic.c $(DIR)headers/memory.h
	$(CC) $(OPT) -c $(DIR)mnemonic.c

clean:
	rm *.o
