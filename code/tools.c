/*
 * Copyright (c) 2018 DDRDmakar
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "headers/error.h"
#include "headers/defines.h"
#include "headers/memory.h"
#include "headers/tools.h"

int is_udec_num(char *line)
{
	if (!line || strlen(line) == 0) return 0;
	
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] < '0' || '9' < line[i]) return 0;
	}
	return 1;
}
int is_ubin_num(char *line)
{
	if (!line || strlen(line) == 0) return 0;
	
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] != '0' && line[i] != '1') return 0;
	}
	return 1;
}
int is_uhex_num(char *line)
{
	if (!line || strlen(line) == 0) return 0;
	
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (
			!('0' <= line[i] && line[i] <= '9') &&
			!('a' <= line[i] && line[i] <= 'f') &&
			!('A' <= line[i] && line[i] <= 'F')
		) return 0;
	}
	return 1;
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
			snprintf(strcomment, comment_len, "\'addr %04X\' ", (unsigned int)i);
			strcat(current_str, strcomment);
		}
		
		if (storage[i])
		{
			// # + 2 chars in opcode + one space + null character
			char *strhex = (char*)malloc(5 * sizeof(char));
			MALLOC_NULL_CHECK(strhex);
			snprintf(strhex, 5, "#%02X ", storage[i]);
			strcat(current_str, strhex);
			free(strhex);
		}
		else strcat(current_str, "0 ");
	}
	return current_str;
}

char *program_memory_to_str(Memory *mem, uint8_t *storage, size_t size)
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
			snprintf(strcomment, comment_len, "\'addr %04X\' ", (unsigned int)i);
			strcat(current_str, strcomment);
		}
		
		if (mem->PM_str && mem->PM_str[i])
		{
			strcat(current_str, mem->PM_str[i]);
			strcat(current_str, " ");
		}
		else
		{
			if (storage[i])
			{
				// # + 2 chars in opcode + one space + null character
				char *strhex = (char*)malloc(5 * sizeof(char));
				MALLOC_NULL_CHECK(strhex);
				snprintf(strhex, 5, "#%02X ", storage[i]);
				strcat(current_str, strhex);
				free(strhex);
			}
			else strcat(current_str, "0 ");
		}
	}
	return current_str;
}

char *uint32_to_hex_str(uint32_t value)
{
	// input - 32 bit unsigned integer
	// allocate 8 symbols for digits + 1 for # + 1 for space + 1 for \0
	size_t result_len = 11;
	char *result = (char*)malloc(result_len * sizeof(char));
	MALLOC_NULL_CHECK(result);
	snprintf(result, result_len, "#%X ", value);
	return result;
}

uint32_t hex_str_to_uint32(char *str)
{
	size_t slen = strlen(str);
	char *newstr = (char*)malloc(slen * sizeof(char));
	MALLOC_NULL_CHECK(newstr);
	strncpy(newstr, str, slen);
	char *newstr_reserve_to_free = newstr;
	
	const char *errstr = "ERROR - wrong hex string format";
	if (!str) progstop(errstr, 1);
	while (newstr[0] && (newstr[0] == ' ' || newstr[0] == '#')) ++newstr;
	while (newstr[0] && newstr[strlen(newstr) - 1] == ' ') newstr[strlen(newstr) - 1] = '\0';
	if (!newstr[0]) return 0;
	
	if (!is_uhex_num(newstr)) progstop(errstr, 1);
	
	uint32_t result = strtoul(newstr, NULL, 16);
	free(newstr_reserve_to_free);
	return result;
}

/*
 * Split text into parts separated by "sep"
 * If sep or str is NULL or empty, function returns NULL
 */
char** text_split(const char *str, const char *sep)
{
	if (!str || !sep) progstop("Text pointer or sep passed for split is NULL", 1);
	
	size_t result_write_pointer = 0; // position in "result" array to write pointers into
	
	unsigned int text_length = strlen(str);
	unsigned int sep_length = strlen(sep);
	if (!text_length || !sep_length) return NULL;
	
	// resulting strings array
	char **result = (char**)calloc(text_length+1, sizeof(char*)); // +1 if text length is 1
	MALLOC_NULL_CHECK(result)
	
	// Current separator and next
	char *current_sep_pointer, *next_sep_pointer;
	
	current_sep_pointer = strstr(str, sep); // Find separator
	bool first = (current_sep_pointer != str); // If there is something before first separator
	
	if (!current_sep_pointer) // If there are no separators
	{
		char *current_part = (char*)malloc(text_length+1 * sizeof(char)); // +1 for null terminator
		MALLOC_NULL_CHECK(current_part);
		strncpy(current_part, str, text_length); // copy all text into resulting string
		
		result[result_write_pointer++] = current_part; // Write resulting string into array
	}
	else // If separators were found
	{
		while(current_sep_pointer && result_write_pointer < text_length)
		{
			// Find next separator
			next_sep_pointer = strstr(current_sep_pointer + sep_length, sep);
			
			size_t current_part_len;
			if      (first)            current_part_len = current_sep_pointer - str;
			else if (next_sep_pointer) current_part_len = next_sep_pointer - current_sep_pointer - sep_length;
			else                       current_part_len = str - current_sep_pointer + text_length - sep_length;
			current_part_len /= sizeof(char);
			
			char *current_part = (char*)malloc(current_part_len+1 * sizeof(char)); // +1 for null terminator
			MALLOC_NULL_CHECK(current_part);
			
			strncpy(
				current_part, 
				first ? (str) : (current_sep_pointer + sep_length), 
				current_part_len
			);
			
			
			result[result_write_pointer++] = current_part; // Write resulting string into array
			
			if (!first) current_sep_pointer = next_sep_pointer; // Switch to next separator
			first = false; // Now we end with first loop cycle
		}
	}
	
	return result;
}

// Removes from string doubled " " "tab" and " " "tab" in the begining and end of string
void remove_doubled_spaces(char *str)
{
	bool afterspace = true;
	size_t text_length = strlen(str);
	char *write_pointer = str;
	char *read_pointer = str;
	
	size_t i;
	// iterate through each symbol
	for(i = 0; i <= text_length; ++i)
	{
		if (*read_pointer == ' ' || *read_pointer == '\t' || *read_pointer == '\n')
		{
			if (afterspace)
			{
				// If we have space after space, increment only read pointer
				++read_pointer;
				continue;
			}
			afterspace = true;
			*write_pointer = ' '; // Replace \t and \n with spaces
		}
		else *write_pointer = *read_pointer; // Write data
		
		if (*write_pointer == '\0')     // If it is end of string, exit
		{
			// if string is not empty and it is afterspace here, delete space in the end
			if (write_pointer != str && afterspace) *(write_pointer - 1) = '\0';
			break;
		}
		
		if (*read_pointer != ' ' && *read_pointer != '\t' && *read_pointer != '\n') afterspace = false;
		
		++read_pointer;
		++write_pointer;
	}
}

