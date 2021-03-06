#include <stdio.h>
#include <inttypes.h>

int binary(double d) 
{
	uint64_t  mask,
		  nan_mask = (~0lu << 52)>> 1,
		  inf_mask = (~0lu << 53)>> 1,
		  neg_inf_mask = ~0lu << 52;
	void *p = &d;	
	int i = 0,sign;
	double por = 0. , man = 0.;
	uint64_t *n = (uint64_t*)p;
	if (nan_mask == *n)
	{
		printf("NaN\n");
		return 0;
	}
	if (inf_mask == *n)
	{
		printf("positive infinity\n");
		return 0;
	}
	if (neg_inf_mask == *n)
	{
		printf("negative infinity\n");
		return 0;
	}
	for(mask = ~((~0lu)>>1); mask != 0; mask >>= 1) 
	{
		putchar(mask & *n ? '1' : '0');
		i++;
		if(i == 1) sign = mask & *n ? 1:0;
		if(i > 1 && i < 13 && mask & *n) por += 1 << (12 - i);
		if(i > 12 && mask & *n && i < 40) man += (1.0/(1 << (i - 12)));
		else man += (1.0/((double)(1 << 28)*(double)(1 << (i % 40))));
		if(i%4 == 0) putchar(' '); 
	}
	putchar('\n');
	printf("%lf = %c%lf x 2^(%.0lf - 1023)\n",d,sign ? '-':'+',1+man,por); 
	return 0;
}

int main() 
{
	double d;
	scanf("%lf",&d);
	binary(d);
	return 0;
}
