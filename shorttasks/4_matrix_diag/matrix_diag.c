#include <stdio.h>
#include <string.h>
#define K 16

int matrix_diag(char* fname) 
{
	FILE *f;
	char m[7] = {0,0,0,0,0,0,0};
	int n, _n;
	double cur, sum = 0.;
	
	if((f = fopen(fname, "r")) == NULL) return 1;
	if(fread(&m, sizeof(char), 6, f)!= 6) 
	{
		fclose(f);
		return 1;
	}

	if(strcmp(m, "MATRIX")) 
	{
		fclose(f);
		return 1;
	}

	if(fread(&n, sizeof(int), 1, f) != 1)
	{
		fclose(f);
		return 1;
	}

	for(_n = n; _n; _n--) 
	{
		if(fread(&cur, sizeof(double), 1, f) != 1) 
		{
			fclose(f);
			return 1;
		}
		printf("%lf ", cur);
		sum += cur;
		fseek(f, n*sizeof(double), SEEK_CUR);
	}

	printf("\nsum = %lf\n",sum);
	fclose(f);

	return 0;
}

int create_matrix(char* fname) 
{
	FILE *f;
	double x[K] = {1.0,2.0,3.0,4.0,
		       5.0,6.0,7.0,8.0,
		       9.0,10.0,11.0,12.0,
		       13.0,14.0,15.0,16.0};
	int n = 4;
	char* m = "MATRIX";

	if((f = fopen(fname, "w")) == NULL) return 1;
	if(fwrite(m, sizeof(char),6, f) != 6) 
	{
		fclose(f);
		return 1;
	}

	if(fwrite(&n, sizeof(int), 1, f) != 1)
	{
		fclose(f);
		return 1;
	}

	if(fwrite(x, sizeof(double), K, f) != K) 
	{
		fclose(f);
		return 1;
	}

	fclose(f);

	return 0;
}

int main() 
{
	char* fname = "matrix.bin";

	if(create_matrix(fname))
	{
		printf("failed\n");
		return 0;
	}

	if(matrix_diag(fname)) printf("failed\n");

	return 0;
}
