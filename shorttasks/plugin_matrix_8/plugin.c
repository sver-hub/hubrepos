
void matrix_function(int *n, double** matrix)
{
	int i, j, k = *n;
	double *p = *matrix;
	for(i = 0; i < k; i++) 
	{
		for(j = 0; j < k; j++)
		{
			if(j < i) p[i*k + j] = 0;
		}
	}
}
