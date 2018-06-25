
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include "headers/memory.h"
#include "headers/mnemonic.h"
#include "headers/extvar.h"
#include "headers/error.h"
#include "headers/tools.h"
#include "headers/file.h"
#include "headers/execute.h"
#include "headers/binhex.h"
#include "headers/defines.h"

void setup_extvar(void);
void free_extvar(void);

Extvar *extvar;

void show_help(void) 
{
	char* helpmessage = read_text_file_resources("help.txt");
	printf("%s", helpmessage);
	free(helpmessage);
}

int main(int argc, char** argv)
{
	if (argc < 1 || strlen(argv[0]) == 0) progstop("Error - program started with no arguments", 1);
	
	setup_extvar();
	
	// Current working directory
	// ALLOCATE length of CWD path.
	extvar->CWD = (char*)malloc((MAX_PATH_LENGTH + strlen(argv[0]) + 1) * sizeof(char));
	MALLOC_NULL_CHECK(extvar->CWD);
	extvar->CWD = getcwd(extvar->CWD, (MAX_PATH_LENGTH + 1) * sizeof(char));
	strcat(extvar->CWD, "/");
	
	// Executable location
	extvar->binary_location = (char*)malloc((strlen(argv[0]) + 1) * sizeof(char));
	MALLOC_NULL_CHECK(extvar->binary_location);
	strcpy(extvar->binary_location, argv[0]);
	char *lastslash = strrchr(extvar->binary_location, '/');
	if (lastslash) lastslash[1] = '\0';
	else extvar->binary_location[0] = '\0';
	
	// Resources
	char *str_resources = "resources/";
	size_t resources_location_len = strlen(extvar->binary_location) + strlen(str_resources) + 1;
	extvar->resources_location = (char*)malloc(resources_location_len * sizeof(char));
	MALLOC_NULL_CHECK(extvar->resources_location);
	snprintf(extvar->resources_location, resources_location_len, "%s%s", extvar->binary_location, str_resources);
	
	unsigned convert_file = 0; // If we need to convert file into text/bin
	
	// Parse args
	for (int i = 1; i < argc; ++i)
	{
		int flag_not_last = (i+1 < argc);
		int flag_is_single;
		unsigned int j = 1;
		
		if (strlen(argv[i]) >= 2 && argv[i][0] == '-')
		{
			flag_is_single = 1;
			
			if (argv[i][1] == '-') // Полное написание имени флага
			{
				if      (!strcmp(argv[i], "--help")) { show_help(); return 0; }
				else if (!strcmp(argv[i], "--epm")) extvar->EPM_active = 1;
				else if (!strcmp(argv[i], "--edm")) extvar->EDM_active = 1;
				else if (!strcmp(argv[i], "--debug")) extvar->debug = 1;
				else if (!strcmp(argv[i], "--infile"))  goto ref_flag_infile;
				else if (!strcmp(argv[i], "--outfile")) goto ref_flag_outfile;
				else if (!strcmp(argv[i], "--clk"))     goto ref_flag_clk;
				else if (!strcmp(argv[i], "--verbose")) extvar->verbose = 1;
				else if (!strcmp(argv[i], "--mode"))  goto ref_flag_mode;
				else if (!strcmp(argv[i], "--end"))   goto ref_flag_end;
				else if (!strcmp(argv[i], "--break")) goto ref_flag_break;
				else if (!strcmp(argv[i], "--save"))  goto ref_flag_save;
				else if (!strcmp(argv[i], "--convert")) convert_file = 1; // -z
				// else if ...
				else goto ref_unknown_flag;
				
				goto leave_flag_loop;
			}
			
			flag_is_single = (strlen(argv[i]) == 2);
			if (flag_not_last && strlen(argv[i+1]) == 0) progstop("Error - empty argument.", 1);
			
			for (j = 1; j < strlen(argv[i]); ++j)
			{
				switch (argv[i][j])
				{
					case 'h': { show_help(); return 0;}
					case 'd':
					{
						extvar->debug = 1;
						break;
					}
					case 'i': 
					{
						ref_flag_infile:;
						if (flag_is_single && flag_not_last)
						{
							extvar->input_file_name = argv[i+1];
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - input file name", 1);
						break;
					}
					case 'o': 
					{
						ref_flag_outfile:;
						if (flag_is_single && flag_not_last)
						{
							extvar->output_file_name = argv[i+1];
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - output file name", 1);
						break;
					}
					case 'c':
					{
						ref_flag_clk:;
						if (flag_is_single && flag_not_last)
						{
							if (!is_udec_num(argv[i+1])) progstop("Incorrect argument - MCU clock period", 1);
							extvar->clk = strtoul(argv[i+1], NULL, 10);
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - MCU clock period", 1);
						break;
					}
					case 'v':
					{
						extvar->verbose = 1;
						break;
					}
					case 'm': 
					{
						ref_flag_mode:;
						if (flag_is_single && flag_not_last)
						{
							if (strcmp(argv[i+1], "bin") == 0) extvar->mode = 0;
							else if (strcmp(argv[i+1], "text") == 0) extvar->mode = 1;
							else if (strcmp(argv[i+1], "hex") == 0) extvar->mode = 2;
							else progstop("Incorrect argument - input file type", 1);
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - input file type", 1);
						break;
					}
					case 'e':
					{
						ref_flag_end:;
						if (
							flag_is_single && 
							flag_not_last && 
							is_uhex_num(argv[i+1]) &&
							(1 <= strlen(argv[i+1]) && strlen(argv[i+1]) <= 4)
						)
						{
							extvar->endpoint = (uint16_t)strtoul(argv[i+1], NULL, 16);
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - exitpoint location", 1);
						
						break;
					}
					case 'b':
					{
						// -b ^FF
						// -b _FF
						ref_flag_break:;
						if (
							flag_is_single && 
							flag_not_last &&
							(argv[i+1][0] == '^' || argv[i+1][0] == '_') &&
							(2 <= strlen(argv[i+1]) && strlen(argv[i+1]) <= 5) &&
							is_uhex_num(&argv[i+1][1])
						)
						{
							char *strvalue = &argv[i+1][1];
							if   (argv[i+1][0] == '^') extvar->breakpoints[(uint16_t)strtoul(strvalue, NULL, 16)] = -1;
							else extvar->breakpoints[(uint16_t)strtoul(strvalue, NULL, 16)] = 1;
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - breakpoint location", 1);
						
						break;
					}
					case 's':
					{
						// -s ^FF
						// -s _FF
						ref_flag_save:;
						if (
							flag_is_single && 
							flag_not_last && 
							(argv[i+1][0] == '^' || argv[i+1][0] == '_') &&
							(2 <= strlen(argv[i+1]) && strlen(argv[i+1]) <= 5) &&
							is_uhex_num(&argv[i+1][1])
						)
						{
							char *strvalue = &argv[i+1][1];
							if   (argv[i+1][0] == '^') extvar->savepoints[(uint16_t)strtoul(strvalue, NULL, 16)] = -1;
							else extvar->savepoints[(uint16_t)strtoul(strvalue, NULL, 16)] = 1;
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - savepoint location", 1);
						
						break;
					}
					case 'z':
					{
						convert_file = 1;
						break;
					}
					
					default:
					{
						ref_unknown_flag:;
						const char *err_msg = "Unknown flag %s";
						size_t errlen = strlen(err_msg) + strlen(argv[i]) + 1;
						char *err = (char*)malloc(errlen * sizeof(char));
						MALLOC_NULL_CHECK(err);
						snprintf(err, errlen, err_msg, argv[i]);
						progstop(err, 1);
					}
				}
			}
		}
		leave_flag_loop:;
	}
	
	if (extvar->output_file_name == NULL) extvar->output_file_name = "memory.json";
	if (extvar->input_file_name == NULL) progstop("Error - input file name is not set.", 1);
	
#ifdef _DEBUGINFO
	printf("STRUCTURE OF EXTVAR:\n\tEPM\t%d\n\tEDM\t%d\n\tclk\t%d\n\tdebug\t%d\n\tmode\t%d\n\tverbose\t%d\n\tproduce_file\t%d\n\tlocation\t%s\n\toutput file name\t%s\n\tinput file name\t%s\n",
		extvar->EPM_active, extvar->EDM_active, extvar->clk, extvar->debug, extvar->mode, extvar->verbose, extvar->produce_file, extvar->CWD, extvar->output_file_name, extvar->input_file_name
	);
#endif
	
	Memory m;
	m.PC = 0;
	m.DM_str = NULL;
	m.PM_str = NULL;
	
#ifdef _DEBUGINFO
	// MEMORY TEST
	for (uint16_t i = 0; i < 256; ++i) {m.DM.EDM[i] = i;}
	//m.DM.RDM.ACC = 36;
	// E0 = 224
	printf("#E0 = %d	", m.DM.EDM[0xE0]);
	printf("ACC = %d\n", m.DM.RDM_REG.ACC);
	
	printf("#90 = %d	", m.DM.EDM[0x90]);
	printf("P1 = %d\n", m.DM.RDM_REG.P1);
	
	printf("#A0 = %d	", m.DM.EDM[0xA0]);
	printf("P2 = %d\n", m.DM.RDM_REG.P2);
	
	printf("#B8 = %d	", m.DM.EDM[0xB8]);
	printf("IP = %d\n", m.DM.RDM_REG.IP);
	
	printf("#D0 = %d	", m.DM.EDM[0xD0]);
	printf("PSW = %d\n", m.DM.RDM_REG.PSW.PSW);
	
	printf("#F0 = %d	", m.DM.EDM[0xF0]);
	printf("B = %d\n", m.DM.RDM_REG.B);
#endif
	
	setup_mnemonics_alphabet();
	
	switch (extvar->mode)
	{
		case 0: { setup_memory_bin(&m); break; }
		case 1: { setup_memory_text(&m); break; }
		case 2: { setup_memory_ihex(&m); break; }
		default: { progstop("Error - incorrect simulator mode selected", 1); }
	}
	
	// Convert bin and ihex files into JSON text format
	if (convert_file)
	{
		dump2text(&m);
		return 0;
	}
	
	/* =================== */
	
	execute(&m);
	
	/* =================== */
	
	free_mnemonics_alphabet();
	
	free(extvar);
	
	return 0;
}

void setup_extvar(void)
{
	extvar = (Extvar*)malloc(sizeof(Extvar));
	MALLOC_NULL_CHECK(extvar);
	
	extvar->EDM_active = 0;
	extvar->EPM_active = 0;
	extvar->input_file_name = NULL;
	extvar->output_file_name = NULL;
	extvar->CWD = NULL;
	extvar->debug = 0;
	extvar->verbose = 0;
	extvar->mode = 1; // text mode
	extvar->breakpoints = (int*)calloc(EPM_SIZE, sizeof(int)); // 0 - nothing; -1 - before; 1 - after
	MALLOC_NULL_CHECK(extvar->breakpoints);
	extvar->savepoints = (int*)calloc(EPM_SIZE, sizeof(int));  // 0 - nothing; -1 - before; 1 - after
	MALLOC_NULL_CHECK(extvar->savepoints);
	extvar->ticks = 0;
	extvar->endpoint = -1;
}

void free_extvar(void)
{
	free(extvar->breakpoints);
	free(extvar->savepoints);
	free(extvar->CWD);
	free(extvar);
}
