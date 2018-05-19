#include <string.h>
#include <stdio.h>
#include <jansson.h>

#include "headers/mnemonic.h"
#include "headers/file.h"

void text_2_bin(uint8_t storage[], const char *line, const unsigned int memory_size);
uint8_t detect_mnemonic(char *line);
void push_mnemonic(uint8_t storage[], const unsigned int addr, char *line);
void lowercase(char *line);

void setup_memory(struct Memory *mem)
{
	// Get JSON object of memory
    char *json_text = read_file("memory.json");
	json_error_t error;
    json_t *root = json_loads(json_text, 0, &error);
	// TODO check parse error here
	free(json_text);
	
    json_t *memory_dump = json_object_get(root, "program");
	if (memory_dump == NULL || !json_is_string(memory_dump)) {} /*TODO exit here*/
	char *plain_text = (char*)json_string_value(memory_dump);
	text_2_bin(mem->PM.EPM, plain_text, sizeof(mem->PM.EPM));
	free(memory_dump);
	free(plain_text);
	
    memory_dump = json_object_get(root, "data");
	if (memory_dump == NULL || !json_is_string(memory_dump)) {} /*TODO exit here*/
	plain_text = (char*)json_string_value(memory_dump);
	text_2_bin(mem->DM.EDM, plain_text, sizeof(mem->DM.EDM));
	free(memory_dump);
	free(plain_text);
}

void text_2_bin(uint8_t storage[], const char *line, const unsigned int memory_size)
{
	const unsigned int len = strlen(line);
	
	unsigned int memory_address = 0; // Position to push byte into
	
	char part[256]; // One byte (mnemonic or integer)
	unsigned part_pointer = 0; // "part" array length
	char quote = 0; // If current part of text is commented
	
	for (unsigned i = 0; i < len; ++i) // Iterate through symbols
	{
		if (memory_address >= memory_size) {/*TODO exit here*/}
		
		const char e = line[i]; // Current symbol
		
		if (e == '\'') // If we see quote
		{
			if (part_pointer != 0) // If "part" contains something
			{
				part[part_pointer] = '\0'; // End string
				part_pointer = 0;
				push_mnemonic(storage, memory_address++, part);
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
				push_mnemonic(storage, memory_address++, part);
			}
			continue;
		}
		
		// If we meet beginning of other mnemonic without space
		if ((e == '#' || e == '*') && part_pointer != 0)
		{
			part[part_pointer] = '\0'; // End string
			part_pointer = 0;
			--i;
			push_mnemonic(storage, memory_address++, part);
			continue;
		}
		
		part[part_pointer++] = e; // Add symbol into "part" string
		
		if (part_pointer == 255) // if "part" array is full
		{
			part[part_pointer] = '\0'; // End string
			push_mnemonic(storage, memory_address++, part);
			break;
		}
	}
}

void push_mnemonic(uint8_t storage[], const unsigned int addr, char *line)
{
	uint8_t value = detect_mnemonic(line);
	storage[addr] = value;
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
				printf("ERROR - incorrect mnemonic %s\n", line);
				// TODO exit here
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
					printf("ERROR - incorrect mnemonic %s\n", line);
					// TODO exit here
					return 0;
				}
			}
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 16);
		}
		
		case '*':
		{
			// Check string length
			if (strlen(line) < 2 || 4 < strlen(line))
			{
				printf("ERROR - incorrect mnemonic %s\n", line);
				// TODO exit here
				return 0;
			}
			
			// Check symbols range
			for (unsigned i = 1; i < strlen(line); ++i)
			{
				if (line[i] < '0' && '9' < line[i])
				{
					printf("ERROR - incorrect mnemonic %s\n", line);
					// TODO exit here
					return 0;
				}
			}
			
			char *strvalue = &line[1];
			value = (uint8_t) strtoul(strvalue, NULL, 10);
		}
		
		case '0': {} // Go down
		case '1':
		{
			// Check string length
			if (strlen(line) < 1 || 8 < strlen(line))
			{
				printf("ERROR - incorrect mnemonic %s\n", line);
				// TODO exit here
				return 0;
			}
			
			// Check symbols range
			for (unsigned i = 1; i < strlen(line); ++i)
			{
				if (line[i] != '0' && line[i] != '1')
				{
					printf("ERROR - incorrect mnemonic %s\n", line);
					// TODO exit here
					return 0;
				}
			}
			
			value = (uint8_t) strtoul(line, NULL, 2);
		}
		
		default:
		{
			value = 0;
			// TODO look in JSON here
		}
	}
	
	return value;
}

void lowercase(char *line)
{
	for (unsigned i = 0; i < strlen(line); ++i)
	{
		if ('A' <= line[i] && line[i] <= 'Z') line[i] += ('z' - 'Z');
	}
}
