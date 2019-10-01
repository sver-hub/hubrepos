#include <stdio.h>
#include <string.h>
#include <math.h>
#define BUFFADD 10


typedef struct vector_t
{
	double x,y,z;
} vector_t;


int read_vector(FILE *f) 
{
	char m[7] = {0,0,0,0,0,0,0};
	short n;
	int i, j, size = 0, index = 0;
	double cur, sum = 0.0;
	vector_t v1, v2;
	double len1, len2;
	
	fseek(f, 0 , SEEK_SET);

	if(fread(&m, sizeof(char), 6, f)!= 6)
		return -1;
	
	if(strcmp(m, "VECTOR")) 
		return -1;

	if(fread(&n, sizeof(short), 1, f) != 1)
		return -1;

	for (i = 0; i < n; i++) 
	{	
		fseek(f, 8, SEEK_SET);
		for (j = 0; j < n; j++)
		{
			fread(&v1.x, sizeof(double), 1, f);
			fread(&v1.y, sizeof(double), 1, f);
			fread(&v1.z, sizeof(double), 1, f);
			fread(&v2.x, sizeof(double), 1, f);
			fread(&v2.y, sizeof(double), 1, f);
			fread(&v2.z, sizeof(double), 1, f);
			len1 = sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);
			len2 = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

			if (len1 > len2) 
			{
				fseek(f, -6 * sizeof(double), SEEK_CUR);
				fwrite(&v2.x, sizeof(double), 1, f);
				fwrite(&v2.y, sizeof(double), 1, f);
				fwrite(&v2.z, sizeof(double), 1, f);
				fwrite(&v1.x, sizeof(double), 1, f);
				fwrite(&v1.y, sizeof(double), 1, f);
				fwrite(&v1.z, sizeof(double), 1, f);
			}
			fseek(f, -3 * sizeof(double), SEEK_CUR);
		}
	}
	return n;
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

	f = fopen(argv[1], "r+");
	if (f == NULL)
	{
		printf("file not opened\n");
		return 1;
	}

	size = read_vector(f);
	if (size < 0)
	{
		printf("error while reading file\n");
		return 1;
	}	

	fclose(f);
	f = fopen(argv[1], "r");
	fseek(f, 8, SEEK_SET);
	for (i = 0; i < size*size; i++)
	{
		if (i % 3 == 0 ) putchar('\n');
		fread(&d, sizeof(double), 1 ,f);
		printf("%f ", d);
	}
	putchar('\n');

	return 0;
}
