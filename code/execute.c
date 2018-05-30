#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "headers/execute.h"
#include "headers/extvar.h"
#include "headers/memory.h"
#include "headers/error.h"
#include "instructions.c"

#define PROG_START 0

void breakpoint(struct Memory *mem);
void snapshot(struct Memory *mem);

int execute(struct Memory *mem)
{
	//uint8_t *prog_memory = extvar->EPM_active ? mem->PM.EPM : mem->PM.RPM;
	size_t prog_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	//uint8_t *data_memory = extvar->EPM_active ? mem->DM.EDM : mem->DM.RDM;
	//size_t data_memory_size = extvar->EPM_active ? EDM_SIZE : RDM_SIZE;
	
	mem->PC = PROG_START;
	while (mem->PC < prog_memory_size)
	{
		if (extvar->debug && extvar->verbose)
		{
			printf("[PC 0x%04x]: %s\n", (unsigned int)mem->PC, mem->PM_str[mem->PC]);
			
			if (mem->PC < RPM_SIZE && extvar->breakpoints[mem->PC] == 1) breakpoint(mem);
			if (mem->PC < RPM_SIZE && extvar->savepoints[mem->PC] == 1) snapshot(mem);
		}
		
		struct Instruction_storage current_instruction = instr[mem->PM.RPM[mem->PC]];
		if (current_instruction.i) current_instruction.i(mem);
		else 
		{
			char *err = (char*)malloc(300 * sizeof(char));
			sprintf(err, "Unknown instruction \"%s\" (OPCODE 0x%02x) at address 0x%04x", mem->PM_str[mem->PC], (unsigned int)mem->PM.EPM[mem->PC], (unsigned int)mem->PC);
			progerr(err);
			return 1;
		}
		
		if (extvar->debug && extvar->verbose)
		{
			if (mem->PC < RPM_SIZE && extvar->breakpoints[mem->PC] == -1) breakpoint(mem);
			if (mem->PC < RPM_SIZE && extvar->savepoints[mem->PC] == -1) snapshot(mem);
			
			usleep(1000 * extvar->clk * current_instruction.n_ticks);
		}
		
		++mem->PC;
	}
	
	return 0;
}

void breakpoint(struct Memory *mem)
{
	printf("========> BREAKPOINT. Press Enter to continue");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }
}

void snapshot(struct Memory *mem)
{
	
}
