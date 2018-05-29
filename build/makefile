# intel 8051 simulator makefile

CC=gcc
DIR=../code/
OPT := -g -lm -Os -std=c11 -pedantic -Wextra -Wall -l jansson
BIN_NAME=sim8051

prog: main.o mnemonic.o file.o error.o execute.o
	$(CC) $(OPT) -o $(BIN_NAME) main.o mnemonic.o file.o error.o execute.o

main.o: $(DIR)main.c $(DIR)headers/memory.h $(DIR)headers/mnemonic.h $(DIR)headers/extvar.h
	$(CC) $(OPT) -c $(DIR)main.c

mnemonic.o: $(DIR)mnemonic.c $(DIR)headers/mnemonic.h $(DIR)headers/memory.h $(DIR)headers/file.h $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)mnemonic.c

file.o: $(DIR)file.c $(DIR)headers/file.h $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)file.c

error.o: $(DIR)error.c $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)error.c

execute.o: $(DIR)execute.c $(DIR)headers/execute.h $(DIR)instructions.c
	$(CC) $(OPT) -c $(DIR)execute.c

clean:
	rm -f *.o $(BIN_NAME)
