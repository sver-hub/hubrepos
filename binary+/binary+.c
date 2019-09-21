#include <stdio.h>
#include <inttypes.h>
#include <math.h>

void binary(double d) {
	uint64_t  mask;
	void *p = &d;	
	int i = 0,sign;
	double por, man;
	uint64_t *n = (uint64_t*)p;
	for(mask = ~((~0lu)>>1); mask != 0; mask >>= 1) {
		putchar(mask & *n ? '1' : '0');
		i++;
		if(i == 1) sign = mask & *n ? 1:0;
		if(i > 1 && i < 13 && mask & *n) por+=pow(2,12-i);
		if(i > 12 && mask & *n) man += pow(2,-(i-12));
		if(i%4 == 0) putchar(' '); 
	}
	putchar('\n');
	printf("%lf = %c%lf x 2^(%.0lf - 1023)\n",d,sign ? '-':'+',1+man,por); 
}

int main() {
	double d;
	scanf("%lf",&d);
	binary(d);
	return 0;
}
