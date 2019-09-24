#include <stdio.h>
#include <string.h>
#define K 16

int matrix_diag(char* fname) 
{
	FILE *f;
	if((f = fopen(fname,"r")) == NULL) return 1;
	char m[7] = {0,0,0,0,0,0,0};
	int n, _n;
	double cur, sum = 0.;
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
	if((f = fopen(fname,"w")) == NULL) return 1;
	double x[K] = {1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.,13.,14.,15.,16.};
	int n = 4;
	char* m = "MATRIX";
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
