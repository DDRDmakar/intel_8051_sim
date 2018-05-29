
#ifndef __EXTVAR__
#define __EXTVAR__

#include <stdint.h>

struct Extvar
{
	unsigned int
		EPM_active, 
		EDM_active,
		clk,
		verbose;
	char
		*location,
		*output_file_name;
};

extern struct Extvar *extvar;

#endif
