#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "headers/mnemonic.h"
#include "headers/file.h"
#include "headers/error.h"
#include "headers/extvar.h"
#include "headers/tools.h"
#include "headers/defines.h"

json_t *mnemo;


uint32_t fill_memory(uint8_t *storage, char ***storage_str, const char *line, const unsigned int memory_size);
void push_word(char **storage, const unsigned int addr, char *line);
int push_mnemonic(uint8_t *storage, const unsigned int addr, char *line);
uint8_t get_mnemonic_from_file(char *name);
int32_t detect_mnemonic(char *line);

void setup_memory_text(Memory *mem)
{
	// Get JSON object of memory
	char *str_state = read_text_file_cwd(extvar->input_file_name);
	if (str_state == NULL)
	{
		const char *err_msg = "Error opening memory file \"%s\"";
		const size_t errlen = strlen(err_msg) + strlen(extvar->input_file_name) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, extvar->input_file_name);
		progstop(err, 1);
	}
	
	size_t l = strlen(str_state); // Replace newlines with spaces (to make json correct)
	for(size_t i = 0; i < l; ++i) if (str_state[i] == '\n' || str_state[i] == '\t') str_state[i] = ' ';
	
	json_error_t error;
	json_t *root = json_loads(str_state, 0, &error);
	if (root == NULL)
	{
		const char *err_msg = "Error parsing memory file \"%s\"";
		const size_t errlen = strlen(err_msg) + strlen(extvar->input_file_name) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, extvar->input_file_name);
		progstop(err, 1);
	}
	
	// PROGRAM
	json_t *memory_dump = json_object_get(root, "program");
	if (memory_dump == NULL || !json_is_string(memory_dump))
		progstop("Error - \"program\" node not found in memory file", 1);
	char *plain_text = (char*)json_string_value(memory_dump);
	if (plain_text == NULL)
		progstop("Error - getting \"program\" node contents", 1);
	
	uint32_t after_last_address;
	if (extvar->EPM_active)
		after_last_address = fill_memory(mem->PM.EPM, &mem->PM_str, plain_text, EPM_SIZE);
	else
		after_last_address = fill_memory(mem->PM.RPM, &mem->PM_str, plain_text, RPM_SIZE);
	free(plain_text);
	if (extvar->endpoint == -1) extvar->endpoint = after_last_address;
	
	// DATA
	memory_dump = json_object_get(root, "data");
	if (memory_dump == NULL || !json_is_string(memory_dump))
		progstop("Error - \"data\" node not found in memory file", 1);
	plain_text = (char*)json_string_value(memory_dump);
	if (plain_text == NULL)
		progstop("Error - getting \"data\" node contents", 1);
	if (extvar->EPM_active)
		fill_memory(mem->DM.EDM, &mem->DM_str, plain_text, EDM_SIZE);
	else
		fill_memory(mem->DM.RDM, &mem->DM_str, plain_text, RDM_SIZE);
	free(plain_text);
	
	free(root);
	
	// Начальное значение, если оно не было задано через файл
	if (mem->DM_str[0x81] == NULL) mem->DM.RDM_REG.SP = 0x07;
}

uint32_t fill_memory(uint8_t *storage, char ***storage_str, const char *line, const unsigned int memory_size)
//void parse_words(char ***storage, const char *line, const unsigned int memory_size)
{
	const unsigned int len = strlen(line);
	
	unsigned int memory_address = 0; // Position to push byte into
	
	char part[MAX_MNEMONIC_LENGTH]; // One byte (mnemonic or integer)
	unsigned part_pointer = 0; // "part" array length
	char quote = 0; // If current part of text is commented
	
	// Allocate memory for storage of mnemonics
	*storage_str = (char**)malloc(memory_size * sizeof(char*));
	MALLOC_NULL_CHECK(storage_str);
	
	for (unsigned i = 0; i < len; ++i) // Iterate through symbols
	{
		if (memory_address >= memory_size)
		{
			// 11 characters in memory size
			const char *err_msg = "Error - memory dump size is bigger than available memort (%d bytes)";
			const size_t errlen = strlen(err_msg) + 11 + 1;
			char *err = (char*)malloc(errlen * sizeof(char));
			MALLOC_NULL_CHECK(err);
			snprintf(err, errlen, err_msg, memory_size);
			progstop(err, 1);
		}
		
		const char e = line[i]; // Current symbol
		
		if (e == '\'') // If we see quote
		{
			if (part_pointer != 0) // If "part" contains something
			{
				part[part_pointer] = '\0'; // End string
				part_pointer = 0;
				push_word(*storage_str, memory_address, part);
				if (push_mnemonic(storage, memory_address++, part)) memory_address--;
			}
			
			quote = !quote; // Change quotation flag
			continue;
		}
		if (quote) continue; // Skip symbol
		
		if (e == ' ' || e == '	' || e == '\n') // If we see space
		{
			if (part_pointer != 0) // If "part" contains something
			{
				part[part_pointer] = '\0'; // End string
				part_pointer = 0;
				push_word(*storage_str, memory_address, part);
				if (push_mnemonic(storage, memory_address++, part)) memory_address--;
			}
			continue;
		}
		
		// If we meet beginning of other mnemonic without space
		if ((e == '#' || e == '*') && part_pointer != 0)
		{
			part[part_pointer] = '\0'; // End string
			part_pointer = 0;
			--i;
			push_word(*storage_str, memory_address, part);
			if (push_mnemonic(storage, memory_address++, part)) memory_address--;
			continue;
		}
		
		part[part_pointer++] = e; // Add symbol into "part" string
		
		if (part_pointer == MAX_MNEMONIC_LENGTH-1) // if "part" array is full
		{
			part[part_pointer] = '\0'; // End string
			push_word(*storage_str, memory_address, part);
			if (push_mnemonic(storage, memory_address++, part)) memory_address--;
			break;
		}
	}
	
	return memory_address; // After last instruction
}

void push_word(char **storage, const unsigned int addr, char *line)
{
	char *newline = (char*)malloc((strlen(line)+1) * sizeof(char));
	MALLOC_NULL_CHECK(newline);
	if (strlen(line) > 0 && (line[0] == '^' || line[0] == '_')) return;
	strcpy(newline, line);
	storage[addr] = newline;
}

int push_mnemonic(uint8_t *storage, const unsigned int addr, char *line)
{
	int32_t value = detect_mnemonic(line);
	switch (value)
	{
		case -1: // _BREAK I
		{
#ifdef _DEBUGINFO
printf("Adding breakpoint before 0x%4x\n", addr);
#endif
			if (addr < (extvar->EPM_active ? RPM_SIZE : EPM_SIZE)) extvar->breakpoints[addr] = -1; // Break before next instruction
			return 1;
			break;
		}
		case -2: // _SAVE I
		{
#ifdef _DEBUGINFO
printf("Adding savepoint before 0x%4x\n", addr);
#endif
			if (addr < (extvar->EPM_active ? RPM_SIZE : EPM_SIZE)) extvar->savepoints[addr] = -1; // Snapshot before next instruction
			return 1;
			break;
		}
		case -3: // I ^BREAK
		{
#ifdef _DEBUGINFO
printf("Adding breakpoint after 0x%4x\n", addr-1);
#endif
			if (addr != 0) extvar->breakpoints[addr-1] = 1; // Break after previous instruction
			return 1;
			break;
		}
		case -4: // I ^SAVE
		{
#ifdef _DEBUGINFO
printf("Adding savepoint after 0x%4x\n", addr-1);
#endif
			if (addr != 0) extvar->savepoints[addr-1] = 1; // Snapshot after previous instruction
			return 1;
			break;
		}
		default: 
		{
			if (value >= 0 && value < 256) storage[addr] = value;
			else
			{
				const char *err_msg = "ERROR - incorrect mnemonic value %d";
				// 11 charachers for address
				const size_t errlen = strlen(err_msg) + 11 + 1;
				char *err = (char*)malloc(errlen * sizeof(char));
				MALLOC_NULL_CHECK(err);
				snprintf(err, errlen, err_msg, value);
				progstop(err, 1);
			}
			return 0;
		}
	}
	return 0;
}

uint8_t get_mnemonic_from_file(char *name)
{
	if (mnemo == NULL) progstop("Error - mnemonics alphabet config was not set set up", 1);
	
	char *name_copy = (char*)malloc((strlen(name) + 1) * sizeof(char));
	MALLOC_NULL_CHECK(name_copy);
	strcpy(name_copy, name);
	lowercase(name_copy);
	
	json_t *current_mnemonic = json_object_get(mnemo, name_copy);
	if (current_mnemonic == NULL || !json_is_integer(current_mnemonic))
	{
		const char *err_msg = "Error - mnemonic \"%s\" not found";
		const size_t errlen = strlen(err_msg) + strlen(name_copy) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, name_copy);
		progstop(err, 1);
	}
	
	free(name_copy);
	
	// Get node contents
	uint8_t value = (uint8_t)json_integer_value(current_mnemonic); // (char*)memory_dump->children->content;
	return value;
}

void error_incorrect_value(char *line)
{
	const char *err_msg = "Error - incorrect value \"%s\"";
	const size_t errlen = strlen(err_msg) + strlen(line) + 1;
	char *err = (char*)malloc(errlen * sizeof(char));
	MALLOC_NULL_CHECK(err);
	snprintf(err, errlen, err_msg, line);
	progstop(err, 1);
}

int32_t detect_mnemonic(char *line)
{
	if (strlen(line) == 0)
	{
		return 0; // NOP
	}
	
	if (line[0] != '0' && line[0] != '1' && strlen(line) < 2) error_incorrect_value(line);
	
	int32_t value;
	
	switch (line[0])
	{
		case '#':
		{
			// Check string length
			if (strlen(line) < 2 || 3 < strlen(line)) error_incorrect_value(line);
			
			// Check symbols range
			if (!is_uhex_num(line+1)) error_incorrect_value(line);
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 16);
			break;
		}
		
		case '*':
		{
			// Check string length
			if (strlen(line) < 2 || 4 < strlen(line)) error_incorrect_value(line);
			
			// Check symbols range
			if (!is_udec_num(line+1)) error_incorrect_value(line);
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 10);
			break;
		}
		
		case '0': {} // Go down
		case '1':
		{
			// Check string length and symbols range
			if (strlen(line) < 1 || 8 < strlen(line) || !is_ubin_num(line)) error_incorrect_value(line);
			
			value = (uint8_t) strtoul(line, NULL, 2);
			break;
		}
		
		case '_':
		{
			value = 0;
			
			char *line_copy = (char*)malloc((strlen(line) + 1) * sizeof(char));
			MALLOC_NULL_CHECK(line_copy);
			strcpy(line_copy, line);
			lowercase(line_copy);
			
			if      (strcmp(line_copy, "_break") == 0) value = -1; // Break before next instruction
			else if (strcmp(line_copy, "_save") == 0) value = -2; // Snapshot before next instruction
			else error_incorrect_value(line);
			
			free(line_copy);
			
			break;
		}
		
		case '^':
		{
			value = 0;
			
			char *line_copy = (char*)malloc((strlen(line) + 1) * sizeof(char));
			MALLOC_NULL_CHECK(line_copy);
			strcpy(line_copy, line);
			lowercase(line_copy);
			
			if      (strcmp(line_copy, "^break") == 0) value = -3; // Break after previous instruction
			else if (strcmp(line_copy, "^save") == 0) value = -4; // Snapshot after previous instruction
			else error_incorrect_value(line);
			
			free(line_copy);
			
			break;
		}
		
		default:
		{
			value = get_mnemonic_from_file(line);
		}
	}
	
	return (int32_t)value;
}

void lowercase(char *line)
{
	size_t l = strlen(line);
	for (unsigned i = 0; i < l; ++i)
	{
		if ('A' <= line[i] && line[i] <= 'Z') line[i] += ('z' - 'Z');
	}
}

void setup_mnemonics_alphabet(void)
{
	const char *filename = "mnemonics.json";
	const size_t config_location_len = strlen(extvar->resources_location) + strlen(filename) + 1;
	char *config_location = (char*)malloc(config_location_len * sizeof(char));
	MALLOC_NULL_CHECK(config_location);
	snprintf(config_location, config_location_len, "%s%s", extvar->resources_location, filename);
	
	json_error_t error;
	mnemo = json_load_file(config_location, 0, &error);
	
	if (mnemo == NULL) 
	{
		const char *err_msg = "Error opening mnemonics config file \"%s\"";
		const size_t errlen = strlen(err_msg) + strlen(config_location) + 1;
		char *err = (char*)malloc(errlen * sizeof(char));
		MALLOC_NULL_CHECK(err);
		snprintf(err, errlen, err_msg, config_location);
		progstop(err, 1);
	}
}

void free_mnemonics_alphabet(void)
{
	free(mnemo);
}
