#include <stdint.h>
#include <stdlib.h>

#include "headers/execute.h"
#include "headers/extvar.h"
#include "headers/memory.h"
#include "instructions.c"

#define PROG_START 0

void (*instr[256])(struct Memory*) = 
{
	
};

int execute(struct Memory *mem)
{
	uint8_t *prog_memory = extvar->EPM_active ? mem->PM.EPM : mem->PM.RPM;
	size_t prog_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	uint8_t *data_memory = extvar->EPM_active ? mem->DM.EDM : mem->DM.RDM;
	size_t data_memory_size = extvar->EPM_active ? EDM_SIZE : RDM_SIZE;
	
	mem->PC = PROG_START;
	while (mem->PC < prog_memory_size)
	{
		
		++mem->PC;
	}
	
	return 0;
}
