#include <stdio.h>
#include <string.h>
#define K 17

int matrix_diag(FILE* f) 
{
	char m[7] = {0,0,0,0,0,0,0};
	int n, cur,_n;
	if(fread(&m, sizeof(char), 6, f)!= 6) return 1;
	if(strcmp(m, "MATRIX")) return 1;
	if(fread(&n, sizeof(int), 1, f) != 1) return 1;
	for(_n = n; _n; _n--) 
	{
		if(fread(&cur, sizeof(int), 1, f) != 1) return 1;
		printf("%d ", cur);
		fseek(f, n*sizeof(int), SEEK_CUR);
	}
	putchar('\n');
	return 0;
}

int create_file() 
{
	FILE *n;
	n = fopen("file.bin","w");
	int x[K] = {4,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
	char* m = "MATRIX";
	if(fwrite(m, sizeof(char),6, n) != 6) return 1;
	if(fwrite(x, sizeof(int), K, n) != K) return 1;
	fclose(n);
	return 0;
}

int main() 
{
	if(create_file())
	{
		printf("failed\n");
		return 0;
	}
	FILE *f;
	f = fopen("file.bin" , "r");
	if(matrix_diag(f)) printf("failed\n");
	fclose(f);
	return 0;
}
