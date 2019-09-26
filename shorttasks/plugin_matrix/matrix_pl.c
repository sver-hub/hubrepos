#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>

typedef struct matrix
{
	int n;
	double* p;
} matrix;

int get_matrix(char* fname, matrix *mat) 
{	
	FILE* f;
	int k;
	char m[7] = {0,0,0,0,0,0,0};
	void* tmp;

	if((f = fopen(fname,"r")) == NULL) return 1;

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

	if(fread(&k, sizeof(int), 1, f) != 1) 
	{
		fclose(f);
		return 1;
	}
	
	mat->n = k;
	tmp = (double*)malloc((k * k) * sizeof(double));
	if(tmp == NULL) 
	{
		fclose(f);
		return 1;
	}

	mat->p = tmp;
	if(fread(mat->p, sizeof(double), k*k, f) != k*k) 
	{
		fclose(f);
		return 1;
	}

	fclose(f);
	return 0;
}

int put_matrix(char *fname, matrix *mat) 
{
	FILE *f;
	int i, k = mat->n;
	double d;
	char* m = "MATRIX";

	if((f = fopen(fname, "w")) == NULL) return 1;

	if(fwrite(m, sizeof(char),6, f) != 6) 
	{
		fclose(f);
		return 1;
	}

	if(fwrite(&k, sizeof(int), 1, f) != 1) 
	{
		fclose(f);
		return 1;
	}

	if(fwrite(mat->p, sizeof(double), (k * k), f) != (k * k))
	{
		fclose(f);
		return 1;
	}
	
	free(mat->p);
	mat->n = 0;
	mat->p = NULL;
	fclose(f);
	return 0;
}

int create_random_matrix(matrix *mat)
{
	double d;
	int i, m = mat->n;
	void* tmp;

	tmp = (double*)malloc((m * m)*sizeof(double));
	if (tmp == NULL) return 1;
	mat->p = tmp;
	
	for(i = 0; i < (m * m); i++)
	{
		d = (rand() + 0.)/rand();
		mat->p[i] = d;
	}

	return 0;
}

int print_matrix(matrix *mat)
{
	int  i, j, m = mat->n, k;
	double d;

	putchar('\n');
	for(i = 0; i < m; i++)
	{
		putchar('|');
		for(j = 0; j < m; j++)
		{
			k = i*m + j;
			d = mat->p[k];
			printf(" %8.3lf", d);
		}
		printf(" |\n");
	}
	return 0;
}

int main() 
{
	char* fname = "file.bin";
	int n = 0;
	void* lib;
        void (*func)(int *, double **)=NULL;

	matrix mat = {0, NULL};
	srand(time(0));

	scanf("%d", &n);
	mat.n = n;
	if(create_random_matrix(&mat))
	{
		printf("failed creating matrix\n");
		return 0;
	}

	if(put_matrix(fname, &mat)) 
	{	
		printf("failed writing matrix to file\n");
		return 0;
	}

	if(get_matrix(fname, &mat))
	{
		printf("failed reading from file\n");
		return 0;
	}

	if(print_matrix(&mat))
	{
		printf("failed printing\n");
		return 0;
	}
	
	if((lib = dlopen("plugin.so",RTLD_LAZY)) == NULL) 
	{
		printf("lib not found\n");
		return 0;
	}
	
	func = dlsym(lib,"matrix_function");
	if( func == NULL) 
	{
		printf("func not found\n");
		return 0;
	}
	func(&mat.n,&mat.p);
	dlclose(lib);

	print_matrix(&mat);

	free(mat.p);
	return 0;
}

