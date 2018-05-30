# intel 8051 simulator makefile

CC=gcc
DIR=../code/
OPT := -lm -Os -std=gnu11 -pedantic -lpthread -Wextra -Wall -l jansson
BIN_NAME=sim8051

# DEBUG
OPT := -g $(OPT)

prog: main.o mnemonic.o file.o error.o execute.o tools.o
	$(CC) $(OPT) -o $(BIN_NAME) main.o mnemonic.o file.o error.o execute.o tools.o

main.o: $(DIR)main.c $(DIR)headers/memory.h $(DIR)headers/mnemonic.h $(DIR)headers/extvar.h $(DIR)headers/tools.h $(DIR)headers/file.h
	$(CC) $(OPT) -c $(DIR)main.c

mnemonic.o: $(DIR)mnemonic.c $(DIR)headers/mnemonic.h $(DIR)headers/memory.h $(DIR)headers/file.h $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)mnemonic.c

file.o: $(DIR)file.c $(DIR)headers/file.h $(DIR)headers/error.h $(DIR)headers/extvar.h
	$(CC) $(OPT) -c $(DIR)file.c

error.o: $(DIR)error.c $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)error.c

execute.o: $(DIR)execute.c $(DIR)headers/execute.h $(DIR)instructions.c $(DIR)headers/extvar.h $(DIR)headers/memory.h $(DIR)headers/error.h
	$(CC) $(OPT) -c $(DIR)execute.c

tools.o: $(DIR)tools.c $(DIR)headers/tools.h
	$(CC) $(OPT) -c $(DIR)tools.c

clean:
	rm -f *.o $(BIN_NAME)
