
#include "headers/tools.h"
#include <string.h>


int is_udec_num(char *line)
{
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] < '0' || '9' < line[i]) return 0;
	}
	return 1;
}
int is_ubin_num(char *line)
{
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (line[i] != '0' && line[i] != '1') return 0;
	}
	return 1;
}
int is_uhex_num(char *line)
{
	for (size_t i = 0; i < strlen(line); ++i)
	{
		if (
			!('0' <= line[i] && line[i] <= '9') &&
			!('a' <= line[i] && line[i] <= 'f') &&
			!('F' <= line[i] && line[i] <= 'F')
		) return 0;
	}
	return 1;
}
