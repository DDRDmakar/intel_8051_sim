# intel 8051 simulator makefile

CC=gcc
DIR=../code/
OPT := -lm -Os -std=c11 -pedantic -Wextra -Wall -l jansson
BIN_NAME=sim8051

prog: main.o mnemonic.o file.o
	$(CC) $(OPT) -o $(BIN_NAME) main.o mnemonic.o file.o

main.o: $(DIR)main.c $(DIR)headers/memory.h $(DIR)headers/mnemonic.h
	$(CC) $(OPT) -c $(DIR)main.c

mnemonic.o: $(DIR)mnemonic.c $(DIR)headers/mnemonic.h $(DIR)headers/memory.h $(DIR)headers/file.h
	$(CC) $(OPT) -c $(DIR)mnemonic.c

file.o: $(DIR)file.c $(DIR)headers/file.h
	$(CC) $(OPT) -c $(DIR)file.c

clean:
	rm *.o
