#include <stdlib.h>
#include <stdio.h>

#include "headers/error.h"

void errlog(const char *message)
{
	FILE *f = fopen("8051_errorlog.txt", "a");
	if(f == NULL)
	{
		printf("Error creating error log file!\n");
		exit(1);
	}
	fprintf(f, "%s\n", message);
	fclose(f);
}

void progstop(const char *message, const int errcode)
{
	errlog(message);
	fprintf(stderr, "%s\n", message);
	exit(errcode);
}

void progerr(const char *message)
{
	errlog(message);
	fprintf(stderr, "%s\n", message);
}


