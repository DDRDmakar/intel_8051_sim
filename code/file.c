
#include <stdio.h>
#include <stdlib.h>

#include "headers/file.h"
#include "headers/error.h"

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

void write_text_file(const char *filename, const char *str)
{
	FILE *f = fopen(filename, "a");
	if(f == NULL)
	{
		printf("Error creating error log file!\n");
		exit(1);
	}
	fprintf(f, "%s\n", str);
	fclose(f);
}
