#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <time.h>

#include "headers/execute.h"
#include "headers/extvar.h"
#include "headers/memory.h"
#include "headers/error.h"
#include "headers/file.h"
#include "instructions.c"

#define PROG_START 0

void breakpoint(struct Memory *mem);
void snapshot(struct Memory *mem);
void snapshot_end(struct Memory *mem);
void memory_to_file(struct Memory *mem, char *filename);
char *memory_to_str(uint8_t *storage, size_t size);

int execute(struct Memory *mem)
{
	//uint8_t *prog_memory = extvar->EPM_active ? mem->PM.EPM : mem->PM.RPM;
	size_t prog_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	//uint8_t *data_memory = extvar->EPM_active ? mem->DM.EDM : mem->DM.RDM;
	//size_t data_memory_size = extvar->EPM_active ? EDM_SIZE : RDM_SIZE;
	
	mem->PC = PROG_START;
	while (mem->PC < prog_memory_size)
	{
		
		if (mem->PM_str[mem->PC] == NULL)
		{
			if (extvar->debug && extvar->verbose)
			{
				printf("Program ended at 0x%04x\n", (unsigned int)mem->PC);
			}
			break;
		}
		
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
	
	snapshot_end(mem);
	
	return 0;
}

void breakpoint(struct Memory *mem)
{
	(void)mem;
	printf("========> BREAKPOINT. Press Enter to continue");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }
}

void snapshot(struct Memory *mem)
{
	time_t timer;
	char *buffer = (char*)malloc((26 + strlen(extvar->output_file_name) + 1) * sizeof(char));
	struct tm *tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer)/sizeof(char), "%Y_%m_%d__%H_%M_%S__", tm_info);
	strcat(buffer, extvar->output_file_name);
	memory_to_file(mem, buffer);
}

void snapshot_end(struct Memory *mem)
{
	memory_to_file(mem, extvar->output_file_name);
}

void memory_to_file(struct Memory *mem, char *filename)
{
	json_t *root = json_object();
	//json_t *program = json_array();
	//json_t *data = json_array();
	char* program = extvar->EPM_active ? memory_to_str(mem->PM.EPM, EPM_SIZE) : memory_to_str(mem->PM.RPM, RPM_SIZE);
	char* data    = extvar->EDM_active ? memory_to_str(mem->DM.EDM, EDM_SIZE) : memory_to_str(mem->DM.RDM, RDM_SIZE);
	
	json_object_set_new(root, "program", json_string(program));
	json_object_set_new( root, "data", json_string(data));
	char *result = json_dumps(root, 0);
	write_text_file(filename, result);
	free(result);
	json_decref(root);
}

char *memory_to_str(uint8_t *storage, size_t size)
{
	char *current_str = (char*)malloc(size * 10 * sizeof(uint8_t) + 1);
	current_str[0] = '\0';
	
	for (size_t i = 0; i < size; ++i)
	{
		char *strhex = (char*)malloc(4 * sizeof(char));
		if (i % 8 == 0)
		{
			char *strcomment = (char*)malloc(40 * sizeof(char));
			strcomment[0] = '\'';
			strcomment[1] = '\0';
			sprintf(strhex, "addr %02x", (unsigned int)i);
			strcat(strcomment, strhex);
			strcat(strcomment, "' ");
			strcat(current_str, strcomment);
		}
		sprintf(strhex, "#%02x ", storage[i]);
		strcat(current_str, strhex);
	}
	return current_str;
}
