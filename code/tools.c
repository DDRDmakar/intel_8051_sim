
#include <stdlib.h>
#include <stdio.h>

#include "headers/error.h"
#include "headers/tools.h"
#include "headers/defines.h"

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
			snprintf(strcomment, comment_len, "\'addr %04x\' ", (unsigned int)i);
			strcat(current_str, strcomment);
		}
		
		if (storage[i])
		{
			// # + 2 chars in opcode + one space + null character
			char *strhex = (char*)malloc(5 * sizeof(char));
			MALLOC_NULL_CHECK(strhex);
			snprintf(strhex, 5, "#%02x ", storage[i]);
			strcat(current_str, strhex);
			free(strhex);
		}
		else strcat(current_str, "0 ");
	}
	return current_str;
}
