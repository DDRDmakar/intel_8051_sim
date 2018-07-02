#include <8051.h>

#define SIZE 13

void recursive(unsigned char *arr, unsigned int index)
{
	// 0xE4
	if (arr[index] != 228) recursive(arr, index+1);
	else
	{
		P0 = arr[index];
	}
}

void main(void)
{
	unsigned char array[] = {0, 1, 2, 5, 15, 100, 60, 8, 99, 11, 228, 44, 1};
	recursive(array, 0);
}
