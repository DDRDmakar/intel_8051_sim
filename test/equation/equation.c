 
#include <8051.h>
 
void main(void)
{
	/*
	 * 
	 * (-28.8 * (13+5/7) + 6.6 * (2/3)) / ((1+11/16) * -2.25)
	 *  = 102.8665490887713
	 */
    float a = -28.8;
	float b = 13.0 + (5.0/7.0);
	float c = 6.6;
	float d = 2;
	float e = 3;
	
	float a1 = 1;
	float b1 = 11.0 / 16.0;
	float c1 = -2.25;
	
	float result = (a * b + c * (d/e)) / ((a1+b1) * c1);
	
	P0 = (int)result;
}
