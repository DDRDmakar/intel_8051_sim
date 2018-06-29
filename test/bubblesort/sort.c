
#include <8051.h>
#define MAX 16

int list[MAX] = {1, 11, 13, 4, 7, 12, 0, 9, 5, 2, 10, 15, 6, 8, 3, 14};

void bubbleSort() {
	int temp;
	int i,j;
	int swapped = 0;
	
	for(i = 0; i < MAX-1; i++) { 
		swapped = 0;
		
		for(j = 0; j < MAX-1-i; j++) {
			if(list[j] > list[j+1]) {
				temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
				
				swapped = 1;
			}
		}
		
		if(!swapped) {
			break;
		}
	}
	
}

void main() {
	bubbleSort();
}
