
#ifndef __MNEMONIC__
#define __MNEMONIC__

#include <jansson.h>

#include "memory.h"

void setup_memory(struct Memory *mem);
void lowercase(char *line);
void setup_mnemonics_alphabet(void);
void free_mnemonics_alphabet(void);

extern json_t *mnemo;

#define MAX_MNEMONIC_LENGTH 256

#endif
