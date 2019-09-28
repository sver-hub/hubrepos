#include <stdio.h>
#include <stdarg.h>

double sum(int num, ...) 
{
	double sum = 0.;
	int i;
	va_list l;
	va_start(l, num);
	for(i = 0; i < num; i++)
	{
		sum += va_arg(l, double);
	}
	va_end(l);
	return sum;
}

int main() 
{
	double res;
	res = sum(6, 0.5, 1.231, 5., 6.76, 9.3, 2.);
	printf("%lf\n", res);
}
