
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "headers/file.h"
#include "headers/error.h"
#include "headers/extvar.h"

char* read_text_file(const char *filename)
{
	// declare a file pointer
	FILE *infile = fopen(filename, "r");
	
	// quit if the file does not exist
	if (infile == NULL) progstop("Error opening file", 1);
	
	// Get the number of bytes
	fseek(infile, 0L, SEEK_END);
	long numbytes = ftell(infile);
	
	// reset the file position indicator to the beginning of the file
	fseek(infile, 0L, SEEK_SET);	
	
	// grab sufficient memory for the buffer to hold the text
	char *buffer = (char*)calloc(numbytes, sizeof(char));
	
	// memory error
	if(buffer == NULL) progstop("Error allocating memory (file buffer)", 1);
	
	// copy all the text into the buffer */
	fread(buffer, sizeof(char), numbytes, infile);
	fclose(infile);
	
	// return buffer
	return buffer;
}

char* read_text_file_resources(const char *filename)
{
	char *str_resources = "resources/";
	const size_t namelen = strlen(extvar->CWD) + strlen(str_resources) + strlen(filename) + 1;
	char *newfilename = (char*)malloc(namelen * sizeof(char));
	MALLOC_NULL_CHECK(newfilename);
	snprintf(newfilename, namelen, "%s%s%s", extvar->CWD, str_resources, filename);
	return read_text_file(newfilename);
}

void write_text_file(const char *filename, const char *str)
{
	FILE *outfile = fopen(filename, "w");
	if(!outfile) progstop("Error creating file!\n", 1);
	fprintf(outfile, "%s\n", str);
	fclose(outfile);
}

void write_text_file_resources(const char *filename, const char *str)
{
	char *str_resources = "resources/";
	const size_t namelen = strlen(extvar->CWD) + strlen(str_resources) + strlen(filename) + 1;
	char *newfilename = (char*)malloc(namelen * sizeof(char));
	MALLOC_NULL_CHECK(newfilename);
	snprintf(newfilename, namelen, "%s%s%s", extvar->CWD, str_resources, filename);
	write_text_file(newfilename, str);
}
