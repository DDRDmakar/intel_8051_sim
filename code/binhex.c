
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
	
	uint8_t *buffer = (uint8_t*)malloc(program_memory_size * sizeof(uint8_t));
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

void setup_memory_ihex(Memory *mem)
{
#ifdef _DEBUGINFO
	printf("Reading hex file into memory\n");
#endif
	
	const size_t program_memory_size = extvar->EPM_active ? EPM_SIZE : RPM_SIZE;
	
	uint8_t *buffer = (uint8_t*)malloc(program_memory_size * sizeof(uint8_t));
	MALLOC_NULL_CHECK(buffer);
	
	char *ihex_str = read_text_file_cwd(extvar->input_file_name);
	if (!ihex_str || ihex_str[0] != ':')
	{
		const char *err_msg = "Error opening memory file \"%s\"";
		const size_t errlen = strlen(err_msg) + strlen(extvar->input_file_name) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, extvar->input_file_name);
		progstop(err, 1);
	}
	
	char *current_semicolon = ihex_str;
	char *current_line = NULL;
	char length_str[3] = "00";
	char octave_str[3] = "00";
	char offset_str[5] = "0000";
	do
	{
		// 11 bytes min in intel hex line
		if (strlen(current_semicolon) < 11) break;
		length_str[0] = current_semicolon[1];
		length_str[1] = current_semicolon[2];
		if (!is_uhex_num(length_str)) progstop("Error parsing ihex file - incorrect segment length", 1);
		// Allocate memory for ": ** **** ** (current_line_len *) **" + '\0'
		size_t current_line_len = strtoul(length_str, NULL, 16); // Numner of data bytes
		current_line = (char*)malloc((current_line_len + 12 + 1) * sizeof(char));
		MALLOC_NULL_CHECK(current_line);
		strncpy(current_line, current_semicolon, (current_line_len + 12)*2); // Copy line into separate string
		current_line[current_line_len*2 + 11] = '\0'; // End of one line
#ifdef _DEBUGINFO
		printf("Part of intel hex file read: (len %lu)	|%s|\n", current_line_len, current_line);
#endif
		offset_str[0] = current_line[3];
		offset_str[1] = current_line[4];
		offset_str[2] = current_line[5];
		offset_str[3] = current_line[6];
		if (!is_uhex_num(offset_str)) progstop("Error parsing ihex file - incorrect offset", 1);
		uint16_t offset = strtoul(offset_str, NULL, 16);
		if (offset + current_line_len > program_memory_size)
		{
			progstop("Error - hex file is too long. Please enable extra memory in simulator", 1);
		}
		
		if (current_line[7] == '0' && current_line[8] == '1') // If this is end of file
		{
			break;
		}
		if (current_line[7] == '0' && current_line[8] == '0') // If this is data entry
		{
			
			for (size_t i = 0; i < current_line_len; ++i)
			{
				// Place byte of data into little string
				octave_str[0] = current_line[9 +  2*i];
				octave_str[1] = current_line[10 + 2*i];
				uint8_t value = strtoul(octave_str, NULL, 16);
				mem->PM.EPM[offset + i] = value;
			}
		}
		
		free(current_line);
		current_semicolon = strchr(current_semicolon+1, ':');
	}
	while (current_semicolon);
	
	// Set program end point
	if (extvar->endpoint == -1)
	{
		size_t epoint = program_memory_size;
		while (epoint > 0 && mem->PM.EPM[epoint-1] == 0) --epoint;
		extvar->endpoint = epoint;
	}
	
	// Начальное значение, если оно не было задано через файл
	if (mem->DM_str == NULL || mem->DM_str[0x81] == NULL) mem->DM.RDM_REG.SP = 0x07;
	mem->PC = 0x0000;
	
#ifdef _DEBUGINFO
	printf("End reading hex file\n");
#endif
}
