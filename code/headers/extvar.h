
#ifndef __EXTVAR
#define __EXTVAR

typedef struct Extvar
{
	unsigned int
		EPM_active, 
		EDM_active,
		clk,
		debug,
		mode, // 0 - bin; 1 - text;
		verbose,
		produce_file,
		ticks;
	char
		*CWD,
		*input_file_name,
		*output_file_name;
	
	int 
		*breakpoints,  // 0 - nothing; -1 - before; 1 - after
		*savepoints;   // 0 - nothing; -1 - before; 1 - after
} Extvar;

extern Extvar *extvar;

#endif
