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
#include "headers/tools.h"
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
	
	size_t tpc; // Temporary PC
	
	if (extvar->endpoint == -1) progerr("Warning - endpoint was not set manually or automatically.");
	
	mem->PC = PROG_START;
	while (mem->PC < prog_memory_size && mem->PC < (uint32_t)extvar->endpoint)
	{
		tpc = mem->PC;
		
		if (mem->PM_str != NULL && mem->PM_str[tpc] == NULL) break;
		
		Instruction_storage current_instruction = instr[mem->PM.RPM[tpc]];
		
		if (tpc + current_instruction.n_bytes > prog_memory_size) break;
		
		if (current_instruction.i == NULL || current_instruction.n_bytes == 0) 
		{
			const char *err_msg = "Unknown instruction \"%s\" (OPCODE #%02x) at address #%04x";
			// ALLOCATE length of error message + mnemonic length + opcode + PC + 1
			size_t errlen = strlen(err_msg) + (mem->PM_str ? strlen(mem->PM_str[tpc]) : 0) + 2 + 4 + 1;
			char *err = (char*)malloc(errlen * sizeof(char));
			MALLOC_NULL_CHECK(err);
			snprintf(err, errlen, err_msg, (mem->PM_str ? mem->PM_str[tpc] : ""), (unsigned int)mem->PM.EPM[tpc], (unsigned int)tpc);
			progerr(err);
			return 1;
		}
		
		if (extvar->debug)
		{
			if (
				(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && extvar->savepoints[tpc+0] == -1) ||
				(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && extvar->savepoints[tpc+1] == -1) ||
				(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && extvar->savepoints[tpc+2] == -1)
			) snapshot(mem);
			
			if (extvar->verbose)
			{
				if (
					(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && extvar->breakpoints[tpc+0] == -1) ||
					(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && extvar->breakpoints[tpc+1] == -1) ||
					(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && extvar->breakpoints[tpc+2] == -1)
				) breakpoint(mem);
				
				switch (current_instruction.n_bytes)
				{
					case 1:
					{
						printf("[PC #%04x]: #%02x         (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
					case 2:
					{
						printf("[PC #%04x]: #%02x #%02x     (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], mem->PM.EPM[tpc+1], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
					case 3:
					{
						printf("[PC #%04x]: #%02x #%02x #%02x (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], mem->PM.EPM[tpc+1], mem->PM.EPM[tpc+2], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
				}
			}
		}
		
		/* ============ */
		
		current_instruction.i(mem); // Исполнение инструкции
		
		/* ============ */
		
		// Устанавливается флаг паритета
		PSWBITS.P = is_odd_number_of_bits(ACCUM);
		
		if (extvar->debug)
		{
			if (
				(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && extvar->savepoints[tpc+0] == 1) ||
				(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && extvar->savepoints[tpc+1] == 1) ||
				(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && extvar->savepoints[tpc+2] == 1)
			) snapshot(mem);
			
			if (extvar->verbose)
			{
				if (
					extvar->step || (extvar->enable_breakpoints && (
						(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && extvar->breakpoints[tpc+0] == 1) ||
						(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && extvar->breakpoints[tpc+1] == 1) ||
						(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && extvar->breakpoints[tpc+2] == 1)
					))
				) breakpoint(mem);
				
				struct timespec reqtime;
				reqtime.tv_sec = 0;
				reqtime.tv_nsec = (uint32_t)extvar->clk * (uint32_t)(extvar->ticks ? current_instruction.n_ticks : 1) * 1000000;
				
				nanosleep(&reqtime, NULL);
			}
			
		}
		
		IncrPC_1;
	}
	
	if (extvar->debug && extvar->verbose) printf("Program ended at #%04x\n", (unsigned int)mem->PC);
	snapshot_end(mem);
	
	return 0;
}


// 'p' or 'd' + 4 symbols for hex + \n + next symbol + \0
#define BREAKPOINT_BUFFER_LEN 8

void breakpoint(Memory *mem)
{
	char buffer[BREAKPOINT_BUFFER_LEN];
	if (!extvar->step) printf("========> BREAKPOINT: ");
	
	while (1)
	{
		if (!fgets(buffer, BREAKPOINT_BUFFER_LEN, stdin)) break;
		
		if (strlen(buffer) == 0 || (strlen(buffer) == 1 && buffer[0] == '\n')) break;
		if (buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = '\0';
		
		char first = buffer[0];
		// p - program memory
		// d - data memory
		
		// Make snapshot
		if (!strcmp(buffer, "save"))
		{
			snapshot(mem);
			continue;
		}
		
		if ((first == 'p' || first == 'd') && strlen(buffer) > 1 && strlen(buffer) <= 5 && is_uhex_num(&buffer[1]))
		{
			size_t maxaddr = 
				first == 'p' ? 
				(extvar->EPM_active ? EPM_SIZE : RPM_SIZE) : 
				(extvar->EDM_active ? EDM_SIZE : RDM_SIZE);
			
			uint32_t addr = strtoul(&buffer[1], NULL, 16);
			if (addr < maxaddr) printf("value #%02x\n", (first == 'p' ? mem->PM.EPM[addr] : mem->DM.EDM[addr]));
			else printf("Error - address is too big. Try again: ");
		}
		else printf("Error - wrong address. Try again: ");
	}
}

void snapshot(Memory *mem)
{
	// Snapshot counter
	static unsigned int snapshot_counter = 0;
	
	time_t timer;
	// ALLOCATE
	//    35 characters in date + 
	//    11 characters in file number + 
	//    filename length + 
	//    1
	size_t buffer_len = 35 + 11 + strlen(extvar->output_file_name) + 1;
	char *buffer = (char*)malloc(buffer_len * sizeof(char));
	MALLOC_NULL_CHECK(buffer);
	
	// Max 26 characters in date
	char timebuffer[35];
	struct tm *tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, sizeof(timebuffer)/sizeof(char), "%Y_%m_%d__%H_%M_%S", tm_info);
	
	char *last_slash_ptr = strrchr(extvar->output_file_name, '/');
	
	// Allocate string for directory name
	size_t output_file_rel_dir_len = strlen(extvar->output_file_name) + 1;
	char *output_file_rel_dir = (char*)malloc(output_file_rel_dir_len * sizeof(char));
	MALLOC_NULL_CHECK(output_file_rel_dir);
	strncpy(output_file_rel_dir, extvar->output_file_name, output_file_rel_dir_len);
	if (last_slash_ptr) output_file_rel_dir[last_slash_ptr - extvar->output_file_name + 1] = '\0'; // Cut symbols after last slash
	else output_file_rel_dir[0] = '\0';
	
	snprintf(buffer, buffer_len, "%s%s__%d__%s", output_file_rel_dir, timebuffer, snapshot_counter, (last_slash_ptr ? last_slash_ptr+1 : extvar->output_file_name));
	
	if (extvar->debug && extvar->verbose)
	{
		const char *mes_template = "========> Saving machine state into file \"%s\"\n";
		// Allocate length of message + length of filename + 1;
		size_t mes_len = strlen(buffer) + strlen(mes_template) + 1;
		char *mes = (char*)malloc(mes_len * sizeof(char));
		snprintf(mes, mes_len, mes_template, buffer);
		printf("%s", mes);
		free(mes);
	}
	
	memory_to_file(mem, buffer);
	
	free(buffer);
	free(output_file_rel_dir);
	
	++snapshot_counter;
}

void snapshot_end(Memory *mem)
{
	memory_to_file(mem, extvar->output_file_name);
}

void memory_to_file(Memory *mem, char *filename)
{
	json_t *root = json_object();
	char* program = extvar->EPM_active ? memory_to_str(mem->PM.EPM, EPM_SIZE) : memory_to_str(mem->PM.RPM, RPM_SIZE);
	char* data    = extvar->EDM_active ? memory_to_str(mem->DM.EDM, EDM_SIZE) : memory_to_str(mem->DM.RDM, RDM_SIZE);
	
	json_object_set_new(root, "program", json_string(program));
	json_object_set_new(root, "data", json_string(data));
	char *result = json_dumps(root, 0);
	write_text_file_cwd(filename, result);
	free(result);
	free(program);
	free(data);
	json_decref(root);
}


