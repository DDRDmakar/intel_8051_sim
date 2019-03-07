
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <jansson.h>

#include "headers/error.h"
#include "headers/memory.h"
#include "headers/extvar.h"
#include "headers/file.h"
#include "headers/tools.h"
#include "headers/binhex.h"

void setup_memory_bin(Memory *mem)
{
	const size_t program_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	uint8_t *buffer = (uint8_t*)calloc(program_memory_size, sizeof(uint8_t));
	MALLOC_NULL_CHECK(buffer);
	
	buffer = read_bin_file_cwd(buffer, program_memory_size, extvar->input_file_name);
	if (!buffer)
	{
		const char *err_msg = "Error opening memory file \"%s\"";
		const size_t errlen = strlen(err_msg) + strlen(extvar->input_file_name) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, extvar->input_file_name);
		progstop(err, 1);
	}
	
	// Set program end point
	if (extvar->endpoint == -1)
	{
		size_t epoint = program_memory_size;
		while (epoint > 0 && buffer[epoint-1] == 0) --epoint;
		extvar->endpoint = epoint;
	}
	
	for (int i = 0; i < extvar->endpoint; ++i)
	{
		mem->PM.EPM[i] = buffer[i];
	}
	
	free(buffer);
	
	// Начальное значение, если оно не было задано через файл
	if (mem->DM_str == NULL || mem->DM_str[0x81] == NULL) mem->DM.RDM_REG.SP = 0x07;
	mem->PC = 0x0000;
}
