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

#ifndef __TOOLS
#define __TOOLS

#include <stdint.h>
#include <string.h>

int is_udec_num(const char *line);
int is_ubin_num(const char *line);
int is_uhex_num(const char *line);

char *memory_to_str(uint8_t *storage, size_t size);
char *program_memory_to_str(Memory *mem, uint8_t *storage, size_t size);
char *uint32_to_hex_str(uint32_t value);
uint32_t hex_str_to_uint32(char *str);
void remove_doubled_spaces(char *str);
char** text_split(const char *str, const char *sep);

#endif
