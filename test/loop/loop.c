
#include <8051.h>

void main(void)
{
	int value = 7;
	int i;
	
	for (i = 0; i < 4; ++i)
	{
		value *= 2;
	}
	
	
	for (i = 0; i < 22; ++i)
	{
		value += 3;
	}
	
	P0 = value & 0xFF;
	
	// P0 = 0xB2 = 178
}
