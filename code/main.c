#include <string.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	for (int i = 0; i < argc; ++i)
	{
		printf("%s\n", argv[i]);
	}
	
	for (int i = 0; i < argc; ++i)
	{
		if (strlen(argv[i]) >= 2 && argv[i][0] == '-')
		{
			if (argv[i][1] == '-')
			{
				// Полное написание имени флага
			}
			
			for (int j = 1; j < strlen(argv[i]); ++j)
			{
				printf("Processing flag %c\n", argv[i][j]);
				
				switch (argv[i][j])
				{
					case 'h': {}
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
	
	return 0;
}
