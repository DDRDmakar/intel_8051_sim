#include <string.h>
#include <stdio.h>

#include "headers/mnemonic.h"
#include "headers/file.h"
#include "headers/error.h"
#include "headers/extvar.h"

#define MAX_MNEMONIC_LENGTH 256

json_t *mnemo;


void fill_memory(uint8_t storage[], char ***storage_str, const char *line, const unsigned int memory_size);
void push_word(char **storage, const unsigned int addr, char *line);
void push_mnemonic(uint8_t storage[], const unsigned int addr, char *line);
uint8_t get_mnemonic_from_file(char *name);
uint8_t detect_mnemonic(char *line);


void setup_memory(struct Memory *mem)
{
	char *filename = "resources/testdump.json";
	// Get JSON object of memory
	char *str_state = read_text_file(filename);
	if (str_state == NULL)
	{
		char *err = (char*)malloc(100 * sizeof(char));
		sprintf(err, "Error opening memory file \"%s\"", filename);
		progstop(err, 1);
	}
	
	size_t l = strlen(str_state); // Replace newlines with spaces (to make json correct)
	for(size_t i = 0; i < l; ++i) if (str_state[i] == '\n' || str_state[i] == '\t') str_state[i] = ' ';
	lowercase(str_state);
	json_error_t error;
	json_t *root = json_loads(str_state, 0, &error);
	if (root == NULL)
	{
		char *err = (char*)malloc(100 * sizeof(char));
		sprintf(err, "Error parsing memory file \"%s\"", filename);
		progstop(err, 1);
	}
	
	// PROGRAM
	json_t *memory_dump = json_object_get(root, "program");
	if (memory_dump == NULL || !json_is_string(memory_dump))
		progstop("Error - \"program\" node not found in memory file", 1);
	char *plain_text = (char*)json_string_value(memory_dump);
	if (plain_text == NULL)
		progstop("Error - getting \"program\" node contents", 1);
	if (extvar->EPM_active)
		fill_memory(mem->PM.EPM, &mem->PM_str, plain_text, EPM_SIZE);
	else
		fill_memory(mem->PM.RPM, &mem->PM_str, plain_text, RPM_SIZE);
	free(plain_text);
	
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
}

void fill_memory(uint8_t storage[], char ***storage_str, const char *line, const unsigned int memory_size)
//void parse_words(char ***storage, const char *line, const unsigned int memory_size)
{
	const unsigned int len = strlen(line);
	
	unsigned int memory_address = 0; // Position to push byte into
	
	char part[MAX_MNEMONIC_LENGTH]; // One byte (mnemonic or integer)
	unsigned part_pointer = 0; // "part" array length
	char quote = 0; // If current part of text is commented
	
	// Allocate memory for storage
	*storage_str = (char**)malloc(memory_size * sizeof(char*));
	
	for (unsigned i = 0; i < len; ++i) // Iterate through symbols
	{
		if (memory_address >= memory_size)
		{
			char *err = (char*)malloc(100 * sizeof(char));
			sprintf(err, "Error - memory dump size is bigger than available memort (%d bytes)", memory_size);
			progstop(err, 1);
		}
		
		const char e = line[i]; // Current symbol
		
		if (e == '\'') // If we see quote
		{
			if (part_pointer != 0) // If "part" contains something
			{
				part[part_pointer] = '\0'; // End string
				part_pointer = 0;
				push_mnemonic(storage, memory_address, part);
				push_word(*storage_str, memory_address++, part);
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
				push_mnemonic(storage, memory_address, part);
				push_word(*storage_str, memory_address++, part);
			}
			continue;
		}
		
		// If we meet beginning of other mnemonic without space
		if ((e == '#' || e == '*') && part_pointer != 0)
		{
			part[part_pointer] = '\0'; // End string
			part_pointer = 0;
			--i;
			push_mnemonic(storage, memory_address, part);
			push_word(*storage_str, memory_address++, part);
			continue;
		}
		
		part[part_pointer++] = e; // Add symbol into "part" string
		
		if (part_pointer == MAX_MNEMONIC_LENGTH-1) // if "part" array is full
		{
			part[part_pointer] = '\0'; // End string
			push_mnemonic(storage, memory_address, part);
			push_word(*storage_str, memory_address++, part);
			break;
		}
	}
}

void push_word(char **storage, const unsigned int addr, char *line)
{
	char *newline = (char*)malloc((strlen(line)+1) * sizeof(char));
	strcpy(newline, line);
	printf("Pushing \"%s\" into addr %d\n", newline, addr);
	storage[addr] = newline;
}

void push_mnemonic(uint8_t storage[], const unsigned int addr, char *line)
{
	uint8_t value = detect_mnemonic(line);
	storage[addr] = value;
}

uint8_t get_mnemonic_from_file(char *name)
{
	printf("CURRENT MNEMONIC IS: |%s|\n", name);
	
	if (mnemo == NULL) progstop("Error - mnemonics alphabet config was not set set up", 1);
	
	json_t *current_mnemonic = json_object_get(mnemo, name);
	if (current_mnemonic == NULL || !json_is_integer(current_mnemonic))
	{
		char *err = (char*)malloc(100 * sizeof(char));
		sprintf(err, "Error - mnemonic \"%s\" not found", name);
		progstop(err, 1);
	}
	
	// Get node contents
	uint8_t value = (uint8_t)json_integer_value(current_mnemonic); // (char*)memory_dump->children->content;
	printf("MNEMONIC: %s	CODE: %d\n", name, value);
	return value;
}

uint8_t detect_mnemonic(char *line)
{
	if (strlen(line) == 0)
	{
		printf("WARNING - empty mnemonic!");
		return 0; // NOP
	}
	
	uint8_t value;
	
	switch (line[0])
	{
		case '#':
		{
			// Check string length
			if (strlen(line) < 2 || 3 < strlen(line))
			{
				char *err = (char*)malloc(300 * sizeof(char));
				sprintf(err, "ERROR - incorrect value %s", line);
				progstop(err, 1);
				return 0;
			}
			
			// Check symbols range
			for (unsigned i = 1; i < strlen(line); ++i)
			{
				if (
					!('0' <= line[i] && line[i] <= '9') &&
					!('a' <= line[i] && line[i] <= 'f')
				)
				{
					char *err = (char*)malloc(300 * sizeof(char));
					sprintf(err, "ERROR - incorrect value %s", line);
					progstop(err, 1);
					return 0;
				}
			}
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 16);
			break;
		}
		
		case '*':
		{
			// Check string length
			if (strlen(line) < 2 || 4 < strlen(line))
			{
				char *err = (char*)malloc(300 * sizeof(char));
				sprintf(err, "ERROR - incorrect value %s", line);
				progstop(err, 1);
				return 0;
			}
			
			// Check symbols range
			for (unsigned i = 1; i < strlen(line); ++i)
			{
				if (line[i] < '0' || '9' < line[i])
				{
					char *err = (char*)malloc(300 * sizeof(char));
					sprintf(err, "ERROR - incorrect value %s", line);
					progstop(err, 1);
					return 0;
				}
			}
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 10);
			break;
		}
		
		case '0': {} // Go down
		case '1':
		{
			// Check string length
			if (strlen(line) < 1 || 8 < strlen(line))
			{
				char *err = (char*)malloc(300 * sizeof(char));
				sprintf(err, "ERROR - incorrect value %s", line);
				progstop(err, 1);
				return 0;
			}
			
			// Check symbols range
			for (unsigned i = 0; i < strlen(line); ++i)
			{
				if (line[i] != '0' && line[i] != '1')
				{
					char *err = (char*)malloc(300 * sizeof(char));
					sprintf(err, "ERROR - incorrect value %s", line);
					progstop(err, 1);
					return 0;
				}
			}
			
			value = (uint8_t) strtoul(line, NULL, 2);
			break;
		}
		
		default:
		{
			value = get_mnemonic_from_file(line);
		}
	}
	
	return value;
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
	const char *filename = "resources/mnemonics.json";
	json_error_t error;
	mnemo = json_load_file(filename, 0, &error);
	
	if (mnemo == NULL) 
	{
		char *err = (char*)malloc(100 * sizeof(char));
		sprintf(err, "Error opening mnemonics config file \"%s\"", filename);
		progstop(err, 1);
	}
}

void free_mnemonics_alphabet(void)
{
	free(mnemo);
}
