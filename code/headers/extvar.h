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

#ifndef __EXTVAR
#define __EXTVAR

typedef struct Extvar
{
	unsigned int
		EPM_active, 
		EDM_active,
		clk,
		debug,
		mode, // 0 - bin; 1 - text;
		verbose,
		step,
		enable_breakpoints,
		produce_file,
		ticks;
	char
		*CWD,
		*input_file_name,
		*output_file_name,
		*binary_location,
		*resources_location;
	int 
		*breakpoints,  // 0 - nothing; -1 - before; 1 - after
		*savepoints;   // 0 - nothing; -1 - before; 1 - after
	int32_t
		endpoint;
} Extvar;

extern Extvar *extvar;

#endif
