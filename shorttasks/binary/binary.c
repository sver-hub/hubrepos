#include <stdio.h>

void binary(int n) 
{
	unsigned int mask;
	int i = 0;
	for (mask = ~((~0u)>>1); mask != 0; mask >>= 1) 
	{
		putchar(mask & (unsigned int) n ? '1' : '0');
		i++;
		if(!(i % 4) && mask != 1) 
		{
			putchar(i % 8 ? ' ' : '|');
		}
	}
	putchar('\n');
}

int main() 
{
	int n;
	if(scanf("%d", &n) != 1) puts("error");
	else binary(n);
}
