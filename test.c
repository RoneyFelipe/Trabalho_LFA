#include <stdio.h>

int main (void) {
    int num = 2;
	switch(num) {
		case 1: break;
		case 2:
		case 3:
		default: printf("Oiiii\n");
	}
	printf("FIM\n");
}