
#ifndef __MNEMONIC
#define __MNEMONIC

#include <jansson.h>

#include "memory.h"

void setup_memory_text(Memory *mem);
void lowercase(char *line);
void setup_mnemonics_alphabet(void);
void free_mnemonics_alphabet(void);

extern json_t *mnemo;

#endif
