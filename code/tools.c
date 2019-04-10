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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "headers/error.h"
#include "headers/defines.h"
#include "headers/memory.h"
#include "headers/tools.h"

int is_udec_num(const char *line)
{
	if (!line || strlen(line) == 0) return 0;
	
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] < '0' || '9' < line[i]) return 0;
	}
	return 1;
}
int is_ubin_num(const char *line)
{
	if (!line || strlen(line) == 0) return 0;
	
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] != '0' && line[i] != '1') return 0;
	}
	return 1;
}
int is_uhex_num(const char *line)
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
	if (!str) progstop(1, errstr);
	while (newstr[0] && (newstr[0] == ' ' || newstr[0] == '#')) ++newstr;
	while (newstr[0] && newstr[strlen(newstr) - 1] == ' ') newstr[strlen(newstr) - 1] = '\0';
	if (!newstr[0]) return 0;
	
	if (!is_uhex_num(newstr)) progstop(1, errstr);
	
	uint32_t result = strtoul(newstr, NULL, 16);
	free(newstr_reserve_to_free);
	return result;
}
