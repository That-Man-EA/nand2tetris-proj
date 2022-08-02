#include <stdio.h>


int main() {
	int x = 1;
	int sum = 0;
	while(x <= 10) {
	  sum += x++;
	}
	printf("%d\n", sum);

	return 0;
}
