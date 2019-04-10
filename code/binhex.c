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
	if (!buffer) progstop(1, "Error opening memory file \"%s\"", extvar->input_file_name);
	
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
