#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/memory.h"
#include "headers/mnemonic.h"
#include "headers/extvar.h"
#include "headers/error.h"
#include "headers/tools.h"
#include "headers/file.h"
#include "headers/execute.h"

void setup_extvar(void);
void free_extvar(void);

struct Extvar *extvar;

void show_help(void) 
{
	char* helpmessage = read_text_file("resources/help.txt");
	printf("%s", helpmessage);
}

int main(int argc, char** argv)
{
	if (argc < 1 || strlen(argv[0]) == 0) progstop("Error - program started with no arguments", 1);
	
	setup_extvar();
	
	// Executable location
	extvar->location = (char*)malloc(strlen(argv[0]) * sizeof(char));
	strcpy(extvar->location, argv[0]);
	char *lastslash = strrchr(extvar->location, '/');
	if (lastslash) { if (strlen(lastslash) > 1) lastslash[1] = '\0'; }
	else extvar->location[0] = 0;
	
	for (int i = 0; i < argc; ++i)
	{
		printf("%s\n", argv[i]);
	}
	
	// Parse args
	for (int i = 1; i < argc; ++i)
	{
		if (strlen(argv[i]) >= 2 && argv[i][0] == '-')
		{
			if (argv[i][1] == '-') // Полное написание имени флага
			{
				if (strcmp(argv[i], "--help") == 0)
				{
					show_help();
					return 0;
				}
				// else if ...
			}
			
			size_t flag_is_single = (strlen(argv[i]) == 2);
			int flag_not_last = (i+1 < argc);
			
			for (unsigned j = 1; j < strlen(argv[i]); ++j)
			{
				printf("Processing flag %c\n", argv[i][j]);
				
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
						if (flag_is_single && flag_not_last)
						{
							if (strcmp(argv[i+1], "text") == 0) extvar->mode = 1;
							else if (strcmp(argv[i+1], "bin") == 0) extvar->mode = 0;
							else progstop("Incorrect argument - input file type", 1);
							++i;
							goto leave_flag_loop;
						}
						else progstop("Incorrect argument - input file type", 1);
						break;
					}
					case 'b':
					{
						if (flag_is_single && flag_not_last && is_uhex_num(argv[i+1]))
						{
							// TODO add breakpoint here
						}
						else progstop("Incorrect argument - breakpoint location", 1);
						break;
					}
					case 's':
					{
						if (flag_is_single && flag_not_last && is_uhex_num(argv[i+1]))
						{
							// TODO add savepoint here
						}
						else progstop("Incorrect argument - savepoint location", 1);
						break;
					}
					case 'z':
					{
						// TODO produce_file();
						break;
					}
					
					default:
					{
						char *err = (char*)malloc(300 * sizeof(char));
						sprintf(err, "Unknown flag %s", argv[i]);
						progstop(err, 1);
					}
				}
			}
		}
		leave_flag_loop:;
	}
	
	if (extvar->output_file_name == NULL) extvar->output_file_name = "memory";
	if (extvar->input_file_name == NULL) progstop("Error - input file name is not set." ,1);;
	
	printf("STRUCTURE OF EXTVAR:\n\tEPM\t%d\n\tEDM\t%d\n\tclk\t%d\n\tdebug\t%d\n\tmode\t%d\n\tverbose\t%d\n\tproduce_file\t%d\n\tlocation\t%s\n\toutput file name\t%s\n\tinput file name\t%s\n",
		extvar->EPM_active, extvar->EDM_active, extvar->clk, extvar->debug, extvar->mode, extvar->verbose, extvar->produce_file, extvar->location, extvar->output_file_name, extvar->input_file_name
	);
	
	struct Memory m;
	m.PC = 0;
	
	/* MEMORY TEST
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
	*/
	
	setup_mnemonics_alphabet();
	setup_memory(&m);
	execute(&m);
	
	
	free_mnemonics_alphabet();
	
	free(extvar);
	
	return 0;
}

void setup_extvar(void)
{
	extvar = (struct Extvar*)malloc(sizeof(struct Extvar));
	extvar->EDM_active = 0;
	extvar->EPM_active = 0;
	extvar->input_file_name = NULL;
	extvar->output_file_name = NULL;
	extvar->debug = 0;
	extvar->verbose = 0;
	extvar->mode = 1;
	extvar->breakpoints = (int*)calloc(RPM_SIZE, sizeof(int)); // 0 - nothing; -1 - before; 1 - after
	extvar->savepoints = (int*)calloc(RPM_SIZE, sizeof(int));  // 0 - nothing; -1 - before; 1 - after
}

void free_extvar(void)
{
	free(extvar->breakpoints);
	free(extvar->savepoints);
	free(extvar->location);
	free(extvar);
}
