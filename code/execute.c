#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <time.h>
#include <pthread.h>

#include "headers/execute.h"
#include "headers/extvar.h"
#include "headers/memory.h"
#include "headers/error.h"
#include "headers/file.h"
#include "headers/tools.h"
#include "headers/mnemonic.h"
#include "instructions.c"

#define THREAD_ERROR_CREATE -11
#define THREAD_ERROR_JOIN   -12
#define THREAD_SUCCESS        0

#define END_CHECKER_THREAD \
	if (checker_thread_started) \
	{ \
		pthread_cancel(execution_interrupt_checker_thread); \
		thread_status = pthread_join(execution_interrupt_checker_thread, (void**)&thread_status_addr); \
		if (thread_status != THREAD_SUCCESS) progstop("Error joining checker thread", THREAD_ERROR_JOIN); \
		checker_thread_started = 0; \
	}
#define START_CHECKER_THREAD \
	execution_interrupt_checker_args.flag = 0; \
	if (!checker_thread_started && !extvar->step) \
	{ \
		thread_status = pthread_create(          \
			&execution_interrupt_checker_thread, \
			NULL,                                \
			execution_interrupt_checker,         \
			&execution_interrupt_checker_args    \
		);                                       \
		if (thread_status != 0) progstop("Error - unable to create checker thread", THREAD_ERROR_CREATE); \
		checker_thread_started = 1; \
	}
#define ENTER_PRESSED execution_interrupt_checker_args.flag == 1


typedef struct Checker_thread_args
{
	int flag;
} Checker_thread_args;

void breakpoint(Memory *mem);
void snapshot(Memory *mem);
void snapshot_end(Memory *mem);
char* memory_to_str(uint8_t *storage, size_t size);
void* execution_interrupt_checker(void *vargs);

int execute(Memory *mem)
{
	//uint8_t *prog_memory = extvar->EPM_active ? mem->PM.EPM : mem->PM.RPM;
	size_t prog_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	//uint8_t *data_memory = extvar->EPM_active ? mem->DM.EDM : mem->DM.RDM;
	//size_t data_memory_size = extvar->EPM_active ? EDM_SIZE : RDM_SIZE;
	
	size_t tpc; // Temporary PC
	
	if (extvar->endpoint == -1) progerr("Warning - endpoint was not set manually or automatically.");
	
	// Execution interrupt checker thread
	pthread_t execution_interrupt_checker_thread;
	Checker_thread_args execution_interrupt_checker_args;
	int thread_status, thread_status_addr;
	int checker_thread_started = 0;
	
	// Start checker thread
	START_CHECKER_THREAD;
	
	while (mem->PC < prog_memory_size && mem->PC < (uint32_t)extvar->endpoint)
	{
		tpc = mem->PC;
		
		if (extvar->mode == 1 && mem->PM_str != NULL && mem->PM_str[tpc] == NULL) break;
		
		Instruction_storage current_instruction = instr[mem->PM.RPM[tpc]];
		
		if (tpc + current_instruction.n_bytes > prog_memory_size) break;
		
		if (current_instruction.i == NULL || current_instruction.n_bytes == 0) 
		{
			const char *err_msg = "Unknown instruction \"%s\" (OPCODE #%02X) at address #%04X";
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
				(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && (extvar->savepoints[tpc+0] == -1 || extvar->savepoints[tpc+0] == 2)) ||
				(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && (extvar->savepoints[tpc+1] == -1 || extvar->savepoints[tpc+1] == 2)) ||
				(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && (extvar->savepoints[tpc+2] == -1 || extvar->savepoints[tpc+2] == 2))
			) snapshot(mem);
			
			if (extvar->enable_breakpoints && (
				(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && (extvar->breakpoints[tpc+0] == -1 || extvar->breakpoints[tpc+0] == 2)) ||
				(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && (extvar->breakpoints[tpc+1] == -1 || extvar->breakpoints[tpc+1] == 2)) ||
				(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && (extvar->breakpoints[tpc+2] == -1 || extvar->breakpoints[tpc+2] == 2))
			))
			{
				END_CHECKER_THREAD;
				breakpoint(mem);
				START_CHECKER_THREAD;
			}
			
			if (extvar->verbose)
			{
				switch (current_instruction.n_bytes)
				{
					case 1:
					{
						printf("[PC #%04X]: #%02X         (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
					case 2:
					{
						printf("[PC #%04X]: #%02X #%02X     (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], mem->PM.EPM[tpc+1], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
					case 3:
					{
						printf("[PC #%04X]: #%02X #%02X #%02X (%s)\n", (unsigned int)tpc, mem->PM.EPM[tpc], mem->PM.EPM[tpc+1], mem->PM.EPM[tpc+2], (mem->PM_str ? mem->PM_str[tpc] : ""));
						break;
					}
				}
			}
		}
		
		/* ============ */
		
		current_instruction.i(mem); // Исполнение инструкции
		
		/* ============ */
		
		IncrPC_1;
		
		// Устанавливается флаг паритета
		PSWBITS.P = is_odd_number_of_bits(ACCUM);
		
		if (extvar->debug)
		{
			if (
				(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && (extvar->savepoints[tpc+0] == 1 || extvar->savepoints[tpc+0] == 2)) ||
				(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && (extvar->savepoints[tpc+1] == 1 || extvar->savepoints[tpc+1] == 2)) ||
				(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && (extvar->savepoints[tpc+2] == 1 || extvar->savepoints[tpc+2] == 2))
			) snapshot(mem);
			
			// Прекращение исполнения программы по нажатию Enter
			// В режиме step мы не стартуем проверку нажатия enter
			if (!extvar->step && ENTER_PRESSED)
			{
				END_CHECKER_THREAD;
				breakpoint(mem);
				START_CHECKER_THREAD;
			}
			
			else if (
				extvar->step || 
				(extvar->enable_breakpoints && (
					(current_instruction.n_bytes >= 1 && tpc+0 < prog_memory_size && (extvar->breakpoints[tpc+0] == 1 || extvar->breakpoints[tpc+0] == 2)) ||
					(current_instruction.n_bytes >= 2 && tpc+1 < prog_memory_size && (extvar->breakpoints[tpc+1] == 1 || extvar->breakpoints[tpc+1] == 2)) ||
					(current_instruction.n_bytes == 3 && tpc+2 < prog_memory_size && (extvar->breakpoints[tpc+2] == 1 || extvar->breakpoints[tpc+2] == 2))
				))
			)
			{
				END_CHECKER_THREAD;
				breakpoint(mem);
				START_CHECKER_THREAD;
			}
			
			if (extvar->verbose)
			{
				struct timespec reqtime;
				reqtime.tv_sec = extvar->clk / 1000;
				reqtime.tv_nsec = ((uint32_t)extvar->clk % 1000) * (uint32_t)(extvar->ticks ? current_instruction.n_ticks : 1) * 1000000;
				nanosleep(&reqtime, NULL);
			}
			
		}
	}
	
	if (extvar->debug && extvar->verbose) printf("Program ended at #%04X\n", (unsigned int)mem->PC);
	snapshot_end(mem);
	
	// Join checker thread
	END_CHECKER_THREAD;
	
	return 0;
}


// 'p' or 'd' + 4 symbols for hex + \n + next symbol + \0
#define BREAKPOINT_BUFFER_LEN 9

void breakpoint(Memory *mem)
{
	char buffer[BREAKPOINT_BUFFER_LEN] = "";
	
	if (!extvar->step) printf("[PC #%04X]:========> BREAKPOINT: ", mem->PC);
	
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
		
		// on/off step-by-step mode
		if (!strcmp(buffer, "step"))
		{
			extvar->step = extvar->step ? 0 : 1;
			continue;
		}
		
		// Exit simulator
		if (!strcmp(buffer, "exit") || !strcmp(buffer, "quit") || !strcmp(buffer, "q"))
		{
			exit(0);
		}
		
		if ((first == 'p' || first == 'd') && strlen(buffer) > 1 && strlen(buffer) <= 5 && is_uhex_num(&buffer[1]))
		{
			size_t maxaddr = 
				first == 'p' ? 
				(extvar->EPM_active ? EPM_SIZE : RPM_SIZE) : 
				(extvar->EDM_active ? EDM_SIZE : RDM_SIZE);
			
			uint32_t addr = strtoul(&buffer[1], NULL, 16);
			if (addr < maxaddr) printf("value #%02X\n", (first == 'p' ? mem->PM.EPM[addr] : mem->DM.EDM[addr]));
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
	
	if (extvar->debug)
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
	const size_t program_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	const size_t data_memory_size    = extvar->EDM_active ? EDM_SIZE : RDM_SIZE;
	
	json_t *root = json_object();
	
	char *pc_str = uint32_to_hex_str(mem->PC);
	if (!pc_str) progstop("Error converting PC into hex string", 1);
	// PC
	json_object_set_new(root, "PC", json_string(pc_str));
	
	SET_REGISTER_ADDRESSES_ARRAY();
	SET_REGISTER_MNEMO_ARRAY();
	char *register_strings[REGISTERS_COUNT];
	
	for (size_t i = 0; i < REGISTERS_COUNT; ++i)
	{
		register_strings[i] = uint32_to_hex_str(mem->DM.RDM[register_address_array[i]]);
		if (!register_strings[i]) progstop("Error converting register into hex string", 1);
		json_object_set_new(root, register_mnemo_array[i], json_string(register_strings[i]));
	}
	
	char* program = program_memory_to_str(mem, mem->PM.EPM, program_memory_size);
	char* data    = memory_to_str(mem->DM.EDM, data_memory_size);
	json_object_set_new(root, "program", json_string(program));
	json_object_set_new(root, "data", json_string(data));
	
	char *result = json_dumps(root, 0);
	
	write_text_file_cwd(filename, result);
	
	for (size_t i = 0; i < REGISTERS_COUNT; ++i) free(register_strings[i]);
	free(pc_str);
	free(result);
	free(program);
	free(data);
	json_decref(root);
}

void* execution_interrupt_checker(void *vargs)
{
	Checker_thread_args *args = (Checker_thread_args*)vargs;
	
	char buffer[BREAKPOINT_BUFFER_LEN] = "";
	
	while (1)
	{
		// Если нажат Enter
		if (
			!fgets(buffer, BREAKPOINT_BUFFER_LEN, stdin) ||
			strlen(buffer) == 0 || 
			(strlen(buffer) == 1 && buffer[0] == '\n')
		) args->flag = 1;
	}
}
