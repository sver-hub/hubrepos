#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define K 16
int get_zeroed(FILE* f) 
{
	char m[7] = {0,0,0,0,0,0,0};
	int n, cur,_n, i, _i;
        double zero = 0.0, d;
	if(fread(&m, sizeof(char), 6, f)!= 6) return 1;
	if(strcmp(m, "MATRIX")) return 1;
	if(fread(&n, sizeof(int), 1, f) != 1) return 1;
	i = (n % 2) ? (int)(n / 2) + 1 : (int)(n / 2);
	for(_n = n; _n; _n--) 
	{
		for(_i = i; _i; _i--)
		{
			if(fwrite(&zero, sizeof(double),1,f) != 1) return 1;
			fseek(f, sizeof(double), SEEK_CUR);
		}
		if(n % 2) fseek(f, -sizeof(double), SEEK_CUR);
	}
	return 0;
}

int create_matrix(char *fname, int n) 
{
	FILE *f;
	if((f = fopen(fname, "w")) == NULL) return 1;
	int i;
	double d;
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
	for(i = 0; i < n*n; i++)
	{
		d = (rand() + 0.)/rand();
		if(fwrite(&d, sizeof(double), 1, f) != 1) 
		{
			fclose(f);
			return 1; 
		}
	}
	fclose(f);
	return 0;
}

int print_matrix(char* fname)
{
	FILE *f;
	if((f = fopen(fname, "r")) == NULL) return 1;
	char m[7] = {0,0,0,0,0,0,0};
	int n, i, j;
	double d;
	if(fread(&m, sizeof(char), 6, f) != 6) 
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
	putchar('\n');
	for(i = 0; i < n; i++)
	{
		putchar('|');
		for(j = 0; j < n; j++)
		{
			if(fread(&d, sizeof(double), 1, f) != 1) 
			{
				fclose(f);
				return 1;
			}
			printf(" %8.3lf", d);
		}
		printf(" |\n");
	}
	fclose(f);
	return 0;
}

int main() 
{
	FILE *f;
	char* fname = "file.bin";
	int n;
	srand(time(0));
	scanf("%d", &n);
	if(create_matrix(fname,n))
	{
		printf("failed creating matrix\n");
		return 0;
	}
	if(print_matrix(fname))
	{
		printf("failed printing\n");
		return 0;
	}
	f = fopen(fname, "r+");
	get_zeroed(f);
	fclose(f);
	if(print_matrix(fname)) 
	{
		printf("failed printing\n");
		return 0;
	}
	return 0;
}
