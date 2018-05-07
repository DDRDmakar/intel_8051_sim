#include <string.h>
#include <jansson.h>

#include "headers/memory.h"

void text_2_bin(const char cur_line[])
{
	const unsigned int len = strlen(cur_line);
	
	char part[9]; // One byte (mnemonic or integer)
	unsigned short part_pointer = 0; // "part" array length
	char quote = 0; // If current part of text is commented
	
	for (int i = 0; i < len; ++i) // Iterate through symbols
	{
		const char e = cur_line[i]; // Current symbol
		
		if (e == '\'') // If we see quote
		{
			if (part_pointer != 0) // If "part" contains something
			{
				part[part_pointer] = '\0'; // End string
				part_pointer = 0;
				// TODO Call function here
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
				// TODO Call function here
			}
			continue;
		}
		
		if (part_pointer == 8) // If "part" string is full
		{
			part[part_pointer] = '\0'; // End string
			part_pointer = 0;
			// TODO Call function here
			continue;
		}
		
		// If we meet beginning of other mnemonic without space
		if ((e == '#' || e == '*') && part_pointer != 0)
		{
			part[part_pointer] = '\0'; // End string
			part_pointer = 0;
			--i;
			// TODO Call function here
			continue;
		}
		
		part[part_pointer++] = e; // Add symbol into "part" string
	}
}

uint8_t detect_mnemonic(char cur_line[])
{
	return 0;
}
