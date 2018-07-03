/*
 * Copyright (c) 2018 DDRDmakar
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


