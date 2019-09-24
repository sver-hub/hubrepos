#include <stdio.h>
#include <math.h>

double integral(double a, double b, unsigned int n, double (*func)(double))
{
	double result = 0, delta = (b - a)/n;
	int i;
	for(i = 0; i < n; i++) 
	{
		result += func(i*delta)*delta;
	}
	return result;
}

int main() 
{
	printf("result = %lf\n", integral(0, M_PI, 1000,sin));	
	return 0;
}
	
