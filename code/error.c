#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "headers/error.h"

void errlog(const char *message)
{
	time_t timer;
	char buffer[26]; // Max 26 chars in date
	struct tm *tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(buffer, sizeof(buffer)/sizeof(char), "%Y-%m-%d %H:%M:%S", tm_info);
	
	FILE *f = fopen("8051_errorlog.txt", "a");
	if(f == NULL)
	{
		fprintf(stderr, "Error creating error log file!\n");
		exit(1);
	}
	fprintf(f, "%s    %s\n", buffer, message);
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


