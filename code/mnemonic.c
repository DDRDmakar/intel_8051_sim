/*
 * Copyright (c) 2018-2019 DDRDmakar
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "headers/mnemonic.h"
#include "headers/file.h"
#include "headers/error.h"
#include "headers/extvar.h"
#include "headers/memory.h"
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
	SET_REGISTER_ADDRESSES_ARRAY();
	SET_REGISTER_MNEMO_ARRAY();
	
	// Get JSON object of memory
	char *str_state = read_text_file_cwd(extvar->input_file_name);
	if (str_state == NULL) progstop(1, "Error opening memory file \"%s\"", extvar->input_file_name);
	
	size_t l = strlen(str_state); // Replace newlines with spaces (to make json correct)
	for(size_t i = 0; i < l; ++i) if (str_state[i] == '\n' || str_state[i] == '\t') str_state[i] = ' ';
	
	json_error_t error;
	json_t *root = json_loads(str_state, 0, &error);
	if (root == NULL) progstop(1, "Error parsing memory file \"%s\"", extvar->input_file_name);
	
	// PROGRAM
	json_t *memory_dump = json_object_get(root, "program");
	if (memory_dump == NULL || !json_is_string(memory_dump))
		progstop(1, "Error - \"program\" node not found in memory file");
	char *plain_text = (char*)json_string_value(memory_dump);
	if (plain_text == NULL)
		progstop(1, "Error - getting \"program\" node contents");
	
	uint32_t after_last_address;
	if (extvar->EPM_active)
		after_last_address = fill_memory(mem->PM.EPM, &mem->PM_str, plain_text, EPM_SIZE);
	else
		after_last_address = fill_memory(mem->PM.RPM, &mem->PM_str, plain_text, RPM_SIZE);
	if (extvar->endpoint == -1) extvar->endpoint = after_last_address;
	
	// DATA
	memory_dump = json_object_get(root, "data");
	if (memory_dump == NULL || !json_is_string(memory_dump))
		progstop(1, "Error - \"data\" node not found in memory file");
	plain_text = (char*)json_string_value(memory_dump);
	if (plain_text == NULL)
		progstop(1, "Error - getting \"data\" node contents");
	if (extvar->EPM_active)
		fill_memory(mem->DM.EDM, &mem->DM_str, plain_text, EDM_SIZE);
	else
		fill_memory(mem->DM.RDM, &mem->DM_str, plain_text, RDM_SIZE);
	
	// PC
	json_t *reg_pc = json_object_get(root, "PC");
	if (reg_pc == NULL) progstop(1, "Error parsing memory file \"%s\". PC register is not defined.", extvar->input_file_name);
	
	char *reg_pc_str = (char*)json_string_value(reg_pc);
	mem->PC = hex_str_to_uint32(reg_pc_str);
	
	// REGISTERS
	for (size_t i = 0; i < REGISTERS_COUNT; ++i)
	{
		json_t *reg_node = json_object_get(root, register_mnemo_array[i]);
		if (reg_node == NULL) continue;
		char *reg_node_str = (char*)json_string_value(reg_node);
		mem->DM.RDM[register_address_array[i]] = hex_str_to_uint32(reg_node_str);
#ifdef _DEBUGINFO
		printf("MNEMO: %s	ADDR: %x	NODE: |%s|	VALUE: %x\n", register_mnemo_array[i], register_address_array[i], reg_node_str, mem->DM.RDM[register_address_array[i]]);
#endif
	}
	
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
		if (memory_address >= memory_size) progstop(1, "Error - memory dump size is bigger than available memory (%d bytes)", memory_size);
		
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
printf("Adding breakpoint before 0x%04X\n", addr);
#endif
			if (addr < (extvar->EPM_active ? RPM_SIZE : EPM_SIZE))
			{
				if (extvar->breakpoints[addr] == 1) extvar->breakpoints[addr] = 2;
				else extvar->breakpoints[addr] = -1; // Break before next instruction
			}
			return 1;
			break;
		}
		case -2: // _SAVE I
		{
#ifdef _DEBUGINFO
printf("Adding savepoint before 0x%04X\n", addr);
#endif
			if (addr < (extvar->EPM_active ? RPM_SIZE : EPM_SIZE))
			{
				if (extvar->savepoints[addr] == 1) extvar->savepoints[addr] = 2;
				else extvar->savepoints[addr] = -1; // Snapshot before next instruction
			}
			return 1;
			break;
		}
		case -3: // I ^BREAK
		{
#ifdef _DEBUGINFO
printf("Adding breakpoint after 0x%04X\n", addr-1);
#endif
			if (addr != 0)
			{
				if (extvar->breakpoints[addr-1] == -1) extvar->breakpoints[addr-1] = 2;
				else extvar->breakpoints[addr-1] = 1; // Break after previous instruction
			}
			return 1;
			break;
		}
		case -4: // I ^SAVE
		{
#ifdef _DEBUGINFO
printf("Adding savepoint after 0x%04X\n", addr-1);
#endif
			if (addr != 0)
			{
				if (extvar->savepoints[addr-1] == -1) extvar->savepoints[addr-1] = 2;
				else extvar->savepoints[addr-1] = 1; // Snapshot after previous instruction
			}
			return 1;
			break;
		}
		default: 
		{
			if (value >= 0 && value < 256) storage[addr] = value;
			else progstop(1, "ERROR - incorrect mnemonic value %d", value);
			return 0;
		}
	}
	return 0;
}


#define CHECK_IF_MNEMO_INITIALIZED if (mnemo == NULL) progstop(1, "Error - mnemonics alphabet config was not set set up");

uint8_t get_mnemonic_from_file(char *name)
{
	CHECK_IF_MNEMO_INITIALIZED
	
	char *name_copy = (char*)malloc((strlen(name) + 1) * sizeof(char));
	MALLOC_NULL_CHECK(name_copy);
	strcpy(name_copy, name);
	lowercase(name_copy);
	
	json_t *current_mnemonic_2 = json_object_get(mnemo, "textmode");
	json_t *current_mnemonic   = current_mnemonic_2 ? json_object_get(current_mnemonic_2, name_copy) : NULL;
	if (current_mnemonic_2 == NULL || current_mnemonic == NULL || !json_is_integer(current_mnemonic))
	{
		progstop(1, "Error - mnemonic \"%s\" not found", name_copy);
	}
	
	free(name_copy);
	
	// Get node contents
	uint8_t value = (uint8_t)json_integer_value(current_mnemonic); // (char*)memory_dump->children->content;
	return value;
}

const char *get_default_instruction_name(uint8_t instr)
{
	CHECK_IF_MNEMO_INITIALIZED
	
	char str_instr[3];
	snprintf(str_instr, 3, "%02X", instr);
	
	const json_t *current_mnemonic_2 = json_object_get(mnemo, "instrnames");
	const json_t *current_mnemonic   = current_mnemonic_2 ? json_object_get(current_mnemonic_2, str_instr) : NULL;
	
	if (current_mnemonic_2 == NULL || current_mnemonic == NULL || !json_is_string(current_mnemonic))
	{
		progstop(1, "Error - instruction \"%d\" default name not found", instr);
	}
	
	// Get node contents
	const char* value = json_string_value(current_mnemonic); // (char*)memory_dump->children->content;
	return value;
}

void error_incorrect_value(char *line)
{
	progstop(1, "Error - incorrect value \"%s\"", line);
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
		// HEX value
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
		
		// DECIMAL value
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
		
		// BIN value
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
			// Check if it is mnemonic, defined in JSON file
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
	const char *filename = CONFIG_FILE_NAME;
	const size_t config_location_len = strlen(extvar->resources_location) + strlen(filename) + 1;
	char *config_location = (char*)malloc(config_location_len * sizeof(char));
	MALLOC_NULL_CHECK(config_location);
	snprintf(config_location, config_location_len, "%s%s", extvar->resources_location, filename);
	
	json_error_t error;
	mnemo = json_load_file(config_location, 0, &error);
	
	if (mnemo == NULL) progstop(1, "Error opening mnemonics config file \"%s\"", config_location);
}

void free_mnemonics_alphabet(void)
{
	free(mnemo);
}
