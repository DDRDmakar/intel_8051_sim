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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "headers/file.h"
#include "headers/error.h"
#include "headers/extvar.h"

char *read_text_file_internal(const char *filename)
{
	// declare a file pointer
	FILE *infile = fopen(filename, "r");
	
	// quit if the file does not exist
	if (!infile) progstop(1, "Error opening file \"%s\"", filename);
	
	// Get the number of bytes
	fseek(infile, 0L, SEEK_END);
	long numbytes = ftell(infile);
	
	// reset the file position indicator to the beginning of the file
	fseek(infile, 0L, SEEK_SET);	
	
	// grab sufficient memory for the buffer to hold the text
	char *buffer = (char*)calloc(numbytes, sizeof(char));
	
	// memory error
	if (!buffer) progstop(1, "Error allocating memory (text file buffer)");
	
	// copy all the text into the buffer
	fread(buffer, sizeof(char), numbytes, infile);
	fclose(infile);
	
	// return buffer
	return buffer;
}

uint8_t *read_bin_file_internal(uint8_t *destination, const size_t maxlength, const char *filename)
{
	// declare a file pointer
	FILE *infile = fopen(filename, "rb");
	
	// quit if the file does not exist
	if (!infile) progstop(1, "Error opening file \"%s\"", filename);
	
	// Get the number of bytes
	fseek(infile, 0L, SEEK_END);
	size_t numbytes = ftell(infile);
	
	// Check length limit
	if (maxlength < numbytes) numbytes = maxlength;
	
	// reset the file position indicator to the beginning of the file
	fseek(infile, 0L, SEEK_SET);
	
	// copy all the text into the buffer
	fread(destination, sizeof(uint8_t), numbytes, infile);
	fclose(infile);
	
	// return buffer
	return destination;
}

void write_text_file_internal(const char *filename, const char *str)
{
	FILE *outfile = fopen(filename, "w");
	if(!outfile) progstop(1, "Error creating file \"%s\"", filename);
	fprintf(outfile, "%s", str);
	fclose(outfile);
}

void write_bin_file_internal(const char *filename, const uint8_t *buffer, const size_t len)
{
	FILE *outfile = fopen(filename, "wb");
	if(!outfile) progstop(1, "Error creating file \"%s\"", filename);
	fwrite(buffer, sizeof(uint8_t), len, outfile);
	fclose(outfile);
}

//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//

#define READ_TEXT_FILE_AFTER(x) \
	const size_t namelen = strlen(x) + strlen(filename) + 1; \
	char *newfilename = (char*)malloc(namelen * sizeof(char)); \
	MALLOC_NULL_CHECK(newfilename); \
	snprintf(newfilename, namelen, "%s%s", (x), filename); \
	char *result = read_text_file_internal(newfilename); \
	free(newfilename); \
	return result;

char *read_text_file_cwd(const char *filename)
{
	READ_TEXT_FILE_AFTER(extvar->CWD);
}

char *read_text_file_resources(const char *filename)
{
	READ_TEXT_FILE_AFTER(extvar->resources_location);
}

#define READ_BIN_FILE_AFTER(x) \
	const size_t namelen = strlen(x) + strlen(filename) + 1; \
	char *newfilename = (char*)malloc(namelen * sizeof(char)); \
	MALLOC_NULL_CHECK(newfilename); \
	snprintf(newfilename, namelen, "%s%s", (x), filename); \
	uint8_t *result = read_bin_file_internal(destination, maxlength, newfilename); \
	free(newfilename); \
	return result;

uint8_t *read_bin_file_cwd(uint8_t *destination, const size_t maxlength, const char *filename)
{
	READ_BIN_FILE_AFTER(extvar->CWD);
}

uint8_t *read_bin_file_resources(uint8_t *destination, const size_t maxlength, const char *filename)
{
	READ_BIN_FILE_AFTER(extvar->resources_location);
}

#define WRITE_TEXT_FILE_AFTER(x) \
	const size_t namelen = strlen(x) + strlen(filename) + 1; \
	char *newfilename = (char*)malloc(namelen * sizeof(char)); \
	MALLOC_NULL_CHECK(newfilename); \
	snprintf(newfilename, namelen, "%s%s", (x), filename); \
	write_text_file_internal(newfilename, str); \
	free(newfilename);

void write_text_file_cwd(const char *filename, const char *str)
{
	WRITE_TEXT_FILE_AFTER(extvar->CWD);
}

void write_text_file_resources(const char *filename, const char *str)
{
	WRITE_TEXT_FILE_AFTER(extvar->resources_location);
}

#define WRITE_BIN_FILE_AFTER(x) \
	const size_t namelen = strlen(x) + strlen(filename) + 1; \
	char *newfilename = (char*)malloc(namelen * sizeof(char)); \
	MALLOC_NULL_CHECK(newfilename); \
	snprintf(newfilename, namelen, "%s%s", (x), filename); \
	write_bin_file_internal(newfilename, buffer, len); \
	free(newfilename);

void write_bin_file_cwd(const char *filename, const uint8_t *buffer, const size_t len)
{
	WRITE_BIN_FILE_AFTER(extvar->CWD);
}

void write_bin_file_resources(const char *filename, const uint8_t *buffer, const size_t len)
{
	WRITE_BIN_FILE_AFTER(extvar->resources_location);
}
