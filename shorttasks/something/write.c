#include <stdio.h>
#define BUFFADD 10


typedef struct vector_t
{
	double x,y,z;
} vector_t;

int writebbb(FILE *f)
{
	char *vec = "VECTOR";
	short n = 3;
	vector_t v1, v2 , v3;
	v1.x = 2.2;
	v1.y = 3.5;
	v1.z = -1.3;
	v2.x = 5.2;
	v2.y = 8.5;
	v2.z = -4.3;
	v3.x = 6.2;
	v3.y = 5.5;
	v3.z = -0.3;

	fwrite(vec, sizeof(char), 6, f);
	fwrite(&n, sizeof(short), 1, f);
	fwrite(&v1.x, sizeof(double), 1, f);
	fwrite(&v1.y, sizeof(double), 1, f);
	fwrite(&v1.z, sizeof(double), 1, f);
	fwrite(&v2.x, sizeof(double), 1, f);
	fwrite(&v2.y, sizeof(double), 1, f);
	fwrite(&v2.z, sizeof(double), 1, f);
	fwrite(&v3.x, sizeof(double), 1, f);
	fwrite(&v3.y, sizeof(double), 1, f);
	fwrite(&v3.z, sizeof(double), 1, f);
	return 0;
}


int main(int argc, char** argv) 
{
	FILE *f;
	int size, i;
	double d;

	if (argc < 2) 
	{
		printf("not enought arguments\n");
		return 1;
	}

	f = fopen(argv[1], "w");
	if (f == NULL)
	{
		printf("file not opened\n");
		return 1;
	}

	writebbb(f);

	fclose(f);
	
	return 0;
}
