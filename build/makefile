 #
 # Copyright (c) 2018 DDRDmakar
 # 
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions:
 # 
 # The above copyright notice and this permission notice shall be included in all
 # copies or substantial portions of the Software.
 # 
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 # SOFTWARE.
 # 
 #


# intel 8051 simulator makefile

CC=gcc
DIR=../code/
CFLAGS := -lm -Os -std=c11 -pedantic -Wextra -Wall -l pthread -l jansson -D _ENDIANNESS=$(ENDIANNESS)
BIN_NAME=sim8051

ifdef DEBUG
CFLAGS := -g $(CFLAGS) -D _DEBUGINFO
endif

# Endianness == 0 [little-endian]
# Endianness == 1 [big-endian]

ifndef ENDIANNESS
$(error 'ENDIANNESS is undefined. Please define ENDIANNESS=0 (little) or ENDIANNESS=1 (big)')
endif

prog: main.o mnemonic.o file.o error.o execute.o tools.o binhex.o
	$(CC) $(CFLAGS) -o $(BIN_NAME) main.o mnemonic.o file.o error.o execute.o tools.o binhex.o

main.o: $(DIR)main.c $(DIR)headers/memory.h $(DIR)headers/mnemonic.h $(DIR)headers/extvar.h $(DIR)headers/error.h $(DIR)headers/tools.h $(DIR)headers/file.h $(DIR)headers/execute.h $(DIR)headers/binhex.h $(DIR)headers/defines.h
	$(CC) $(CFLAGS) -c $(DIR)main.c

mnemonic.o: $(DIR)mnemonic.c $(DIR)headers/mnemonic.h $(DIR)headers/file.h $(DIR)headers/error.h $(DIR)headers/extvar.h $(DIR)headers/tools.h $(DIR)headers/defines.h $(DIR)headers/memory.h
	$(CC) $(CFLAGS) -c $(DIR)mnemonic.c

file.o: $(DIR)file.c $(DIR)headers/file.h $(DIR)headers/error.h $(DIR)headers/extvar.h
	$(CC) $(CFLAGS) -c $(DIR)file.c

error.o: $(DIR)error.c $(DIR)headers/error.h
	$(CC) $(CFLAGS) -c $(DIR)error.c

# _POSIX_C_SOURCE=200809L
# It allows you to use function that are not part of the standard C library but are part of the POSIX.1 (IEEE Standard 1003.1) standard.
execute.o: $(DIR)execute.c $(DIR)headers/execute.h $(DIR)instructions.c $(DIR)headers/extvar.h $(DIR)headers/memory.h $(DIR)headers/tools.h $(DIR)headers/error.h $(DIR)headers/file.h $(DIR)headers/mnemonic.h
	$(CC) $(CFLAGS) -D _POSIX_C_SOURCE=200809L  -c $(DIR)execute.c

tools.o: $(DIR)tools.c $(DIR)headers/tools.h $(DIR)headers/error.h $(DIR)headers/defines.h $(DIR)headers/memory.h
	$(CC) $(CFLAGS) -c $(DIR)tools.c

binhex.o: $(DIR)binhex.c $(DIR)headers/binhex.h $(DIR)headers/error.h $(DIR)headers/memory.h $(DIR)headers/extvar.h $(DIR)headers/file.h $(DIR)headers/tools.h
	$(CC) $(CFLAGS) -c $(DIR)binhex.c

clean:
	rm -f *.o $(BIN_NAME)
