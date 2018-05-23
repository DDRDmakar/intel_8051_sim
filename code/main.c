#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/memory.h"
#include "headers/mnemonic.h"
#include "headers/extvar.h"

struct Extvar *extvar;

void show_help(void) {}

int main(int argc, char** argv)
{
	extvar = (struct Extvar*)malloc(sizeof(struct Extvar));
	extvar->EDM_active = 1;
	extvar->EPM_active = 1;
	
	for (int i = 0; i < argc; ++i)
	{
		printf("%s\n", argv[i]);
	}
	
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
			}
			
			for (unsigned j = 1; j < strlen(argv[i]); ++j)
			{
				printf("Processing flag %c\n", argv[i][j]);
				
				switch (argv[i][j])
				{
					case 'h': { show_help(); return 0;}
					case 'o': {}
					case 'c': {}
					case 'v': {}
					case 's': {}
					case 'm': {}
					case 'b': {}
					case 'z': {}
					
					default: {}
				}
			}
		}
	}
	
	struct Memory m;
	for (uint16_t i = 0; i < 256; ++i) {m.DM.EDM[i] = i;}
	//m.DM.RDM.ACC = 36;
	// E0 = 224
	printf("#E0 = %d	", m.DM.EDM[0xE0]);
	printf("ACC = %d\n", m.DM.RDM.ACC);
	
	printf("#90 = %d	", m.DM.EDM[0x90]);
	printf("P1 = %d\n", m.DM.RDM.P1);
	
	printf("#A0 = %d	", m.DM.EDM[0xA0]);
	printf("P2 = %d\n", m.DM.RDM.P2);
	
	printf("#B8 = %d	", m.DM.EDM[0xB8]);
	printf("IP = %d\n", m.DM.RDM.IP);
	
	printf("#D0 = %d	", m.DM.EDM[0xD0]);
	printf("PSW = %d\n", m.DM.RDM.PSW);
	
	printf("#F0 = %d	", m.DM.EDM[0xF0]);
	printf("B = %d\n", m.DM.RDM.B);
	
	
	// Распарсить входной файл на кусочки
	// Добыть алфавит мнемоник
	// Перевести текстовые мнемоники в инструкции
	
	setup_mnemonics_alphabet();
	setup_memory(&m);
	
	
	
	
	free_mnemonics_alphabet();
	
	free(extvar);
	
	return 0;
}