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
#include "headers/mnemonic.h"
#include "instructions.c"

#define PROG_START 0

void breakpoint(Memory *mem);
void snapshot(Memory *mem);
void snapshot_end(Memory *mem);
void memory_to_file(Memory *mem, char *filename);
char *memory_to_str(uint8_t *storage, size_t size);

int execute(Memory *mem)
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
			if (extvar->debug && extvar->verbose) printf("Program ended at 0x%04x\n", (unsigned int)mem->PC);
			break;
		}
		
		if (extvar->debug && extvar->verbose)
		{
			printf("[PC 0x%04x]: %s\n", (unsigned int)mem->PC, mem->PM_str[mem->PC]);
			
			if (mem->PC < RPM_SIZE && extvar->breakpoints[mem->PC] == 1) breakpoint(mem);
			if (mem->PC < RPM_SIZE && extvar->savepoints[mem->PC] == 1) snapshot(mem);
		}
		
		Instruction_storage current_instruction = instr[mem->PM.RPM[mem->PC]];
		
		/* ============ */
		
		if (current_instruction.i) current_instruction.i(mem); // Исполнение инструкции
		
		/* ============ */
		
		else 
		{
			const char *err_msg = "Unknown instruction \"%s\" (OPCODE 0x%02x) at address 0x%04x";
			// ALLOCATE length of error message + mnemonic length + opcode + PC + 1
			size_t errlen = strlen(err_msg) + strlen(mem->PM_str[mem->PC]) + 2 + 4 + 1;
			char *err = (char*)malloc(errlen * sizeof(char));
			MALLOC_NULL_CHECK(err);
			snprintf(err, errlen, err_msg, mem->PM_str[mem->PC], (unsigned int)mem->PM.EPM[mem->PC], (unsigned int)mem->PC);
			progerr(err);
			return 1;
		}
		
		// Устанавливается флаг паритета
		PSWBITS.P = is_odd_number_of_bits(ACCUM);
		
		if (extvar->debug && extvar->verbose)
		{
			if (mem->PC < RPM_SIZE && extvar->breakpoints[mem->PC] == -1) breakpoint(mem);
			if (mem->PC < RPM_SIZE && extvar->savepoints[mem->PC] == -1) snapshot(mem);
			
			struct timespec reqtime;
			reqtime.tv_sec = 0;
			reqtime.tv_nsec = extvar->clk * current_instruction.n_ticks * 1000000;
			nanosleep(&reqtime, NULL);
		}
		
		IncrPC_1;
	}
	
	snapshot_end(mem);
	
	return 0;
}

void breakpoint(Memory *mem)
{
	(void)mem;
	printf("========> BREAKPOINT. Press Enter to continue");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }
}

void snapshot(Memory *mem)
{
	// Snapshot counter
	static unsigned int snapshot_counter = 0;
	
	time_t timer;
	// ALLOCATE
	//    CWD length +
	//    leng
	//    26 characters in date + 
	//    11 characters in file number + 
	//    filename length + 
	//    1
	size_t buffer_len = 26 + 11 + strlen(extvar->output_file_name) + 1;
	char *buffer = (char*)malloc(buffer_len * sizeof(char));
	MALLOC_NULL_CHECK(buffer);
	
	// Max 26 characters in date
	char timebuffer[26];
	struct tm *tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, sizeof(timebuffer)/sizeof(char), "__%Y_%m_%d__%H_%M_%S__", tm_info);
	
	snprintf(buffer, buffer_len, "%d%s%s", snapshot_counter, timebuffer, extvar->output_file_name);
	memory_to_file(mem, buffer);
	
	free(buffer);
	
	++snapshot_counter;
}

void snapshot_end(Memory *mem)
{
	memory_to_file(mem, extvar->output_file_name);
}

void memory_to_file(Memory *mem, char *filename)
{
	json_t *root = json_object();
	//json_t *program = json_array();
	//json_t *data = json_array();
	char* program = extvar->EPM_active ? memory_to_str(mem->PM.EPM, EPM_SIZE) : memory_to_str(mem->PM.RPM, RPM_SIZE);
	char* data    = extvar->EDM_active ? memory_to_str(mem->DM.EDM, EDM_SIZE) : memory_to_str(mem->DM.RDM, RDM_SIZE);
	
	json_object_set_new(root, "program", json_string(program));
	json_object_set_new(root, "data", json_string(data));
	char *result = json_dumps(root, 0);
	write_text_file(filename, result);
	free(result);
	json_decref(root);
}

char *memory_to_str(uint8_t *storage, size_t size)
{
	// Decrease size
	while (size > 0 && storage[size-1] == 0) --size;
	
	// ALLOCATE memory for memory dump string + comments
	// 4 - reserve
	// 14 - max autocomment size
	const size_t current_str_len = size*(MAX_MNEMONIC_LENGTH + 4) + (size/8*14) + 1;
	char *current_str = (char*)malloc(current_str_len * sizeof(char));
	MALLOC_NULL_CHECK(current_str);
	current_str[0] = '\0';
	
	for (size_t i = 0; i < size; ++i)
	{
		if (i % 8 == 0)
		{
			// max comment length is 14
			const size_t comment_len = 14;
			char strcomment[comment_len];
			snprintf(strcomment, comment_len, "\'addr %04x\' ", (unsigned int)i);
			strcat(current_str, strcomment);
		}
		
		if (storage[i])
		{
			// # + 2 chars in opcode + one space + null character
			char *strhex = (char*)malloc(5 * sizeof(char));
			MALLOC_NULL_CHECK(strhex);
			snprintf(strhex, 5, "#%02x ", storage[i]);
			strcat(current_str, strhex);
			free(strhex);
		}
		else strcat(current_str, "0 ");
	}
	return current_str;
}
