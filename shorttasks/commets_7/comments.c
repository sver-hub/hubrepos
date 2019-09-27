#define BUFFADD 100
#define ERROR_MEM -1
#define EOFFF -2
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


ssize_t read_str(FILE* f, char** strp) 
{
	char *buf = NULL;
	size_t size = 0;
	size_t mem = 0;
	char *tmp = NULL;
	int sym = 0;

	while(1)
	{
		sym = fgetc(f);
		if (sym == '\n' || sym == EOF) 
			{
				if(!size) return EOFFF;
				break;
			}

		if (size == mem)
		{
			tmp = (char*)realloc(buf, size + BUFFADD);
			if (tmp == NULL) return ERROR_MEM;
			buf = tmp;
			mem += BUFFADD;
		}
		buf[size] = (char) sym;
		size++;
	}
	
	tmp = (char*)realloc(buf, size + 1);
	if (tmp == NULL) return ERROR_MEM;
	buf = tmp;
	buf[size] = '\0';
	*strp = buf;
	return size;
}

int delcom(char* fname)
{
	FILE *f;
	char *str, *tmp, *towrite = NULL;
	int i, k, quotes = 0, len, alllen = 0;

	if((f = fopen(fname, "r")) == NULL) return 1;

	while(1)
	{
		len = read_str(f, &str);
		if (len == ERROR_MEM) 
		{
			printf("error\n");
			fclose(f);
			return 1;
		}
		else if (len == EOFFF) break;

		for (i = 0; i < len; i++)
		{	
			if (!quotes && str[i] == '\"') quotes = 1;
			else if (quotes && str[i] == '\"') quotes = 0;

			if (str[i] == '/' && str[i+1] == '/' && !quotes) break;
		}

		if(i)
		{
			tmp = (char*)realloc(towrite, (i + 1 + alllen)*sizeof(char));
			if (tmp == NULL) 
				{
					fclose(f);
					return 1;
				}
			for (k = 0; k < i; k ++) tmp[k + alllen] = str[k];

			free(str);
			tmp[i + alllen] = '\n';
			alllen += (i + 1);
			towrite = tmp;
		}
	}
	towrite[alllen - 1] = '\0';

	if ((f = fopen(fname, "w")) == NULL) return 1;
	fputs(towrite, f);
	fclose(f);

	return 0;
}

int write_file(FILE* f)
{
	char* text = "here is some //text bruh\nand then \"some //more\"\nand again// text\n//comment";

	if (fputs(text, f) == EOF) return 1;
	return 0;
}

int main() 
{
	FILE* f;
	char* fname = "test.txt";

	if((f = fopen(fname, "w")) == NULL)
	{
		printf("couldn open file\n");
		return 1;
	}

	if(write_file(f)) 
	{
		printf("couldn`t write file\n");
		return 1;
	}

	fclose(f);

	if(delcom(fname))
	{
		printf("error occured during comment deletion\n");
		return 1;
	}
	return 0;
}

	

