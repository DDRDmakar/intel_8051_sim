
#ifndef __EXTVAR__
#define __EXTVAR__

#include <stdint.h>

struct Extvar
{
	unsigned
		EPM_active, 
		EDM_active;
};

extern struct Extvar *extvar;

#endif
