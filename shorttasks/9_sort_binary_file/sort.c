#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFADD 50
#define MEM_ERROR -1

typedef struct my_string 
{
	char *str;
	int n;
} my_string;

ssize_t fread_line(FILE* f, my_string *line)
{
    char* buff = NULL, *tmp;
    size_t size = 0;
    size_t mem = 0;
    char sym = 0;

    while(1)
    {
        if (fread(&sym, sizeof(char), 1, f) != 1) break;
        if(sym == '\n' || sym == EOF) break;

        if(size == mem)
        {
            tmp = (char*) realloc(buff, (size + BUFFADD) * sizeof(char));
            if(tmp == NULL) return MEM_ERROR;

            buff = tmp;
            mem += BUFFADD;
        }
        buff[size++] = (char)sym;
    }

    tmp = (char*) realloc(buff, (size + 1) * sizeof(char));
    if(tmp == NULL) return MEM_ERROR;

    tmp[size] = '\0';
    line->str = tmp;
    line->n = size;
    return size;
}

ssize_t rd_lines(FILE* f, my_string **lines)
{
    my_string* buff = NULL, *tmp = NULL;
    size_t size = 0, mem = 0;
    my_string line;
    

    while(1)
    {
        if (fread_line(f, &line) < 2) break;

        if(size == mem)
        {
            tmp = (my_string*) realloc(buff, (size + BUFFADD) * sizeof(my_string));
            if(tmp == NULL) return MEM_ERROR;

            buff = tmp;
            mem += BUFFADD;
        }
        buff[size++] = line;
    }

    tmp = (my_string*) realloc(buff, (size + 1) * sizeof(my_string));
    if(tmp == NULL) return MEM_ERROR;

    *lines = tmp;
 
    return size;
}

int cmp_lines(const void* p1, const void* p2)
{
    return strcmp(((my_string*)p1)->str, ((my_string*)p2)->str);
}

int write_file(FILE* f)
{
    char* text = "here is some //text \n\
and then \"some //more\"\n\
and again// text\n\
//comment\n\
testing out// the prog";

    if (fwrite(text, sizeof(char), 93, f) == EOF) return 1;
    return 0;
}


int main(int argc, char** argv)
{
	FILE *f;
	my_string *lines;
	int numlines, i;
	char ent = '\n';


	if (argc < 2)
	{
		printf("file name has to be the first argument\n");
		return 1;
	}

	f = fopen(argv[1], "w");
	write_file(f);
	fclose(f);

	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("couldn`t open the file\n");
		return 1;
	}

	numlines = rd_lines(f, &lines);
	if (numlines == MEM_ERROR)
	{
		printf("memory error\n");
		return 1;
	}

	printf("\n\n__ORIGINAL_FILE___\n");
	for (int i = 0; i < numlines; i++)
	{
		printf("%s\t%d\n", lines[i].str, lines[i].n);
	}

	qsort(lines, numlines, sizeof(my_string), cmp_lines);

	fclose(f);
	f = fopen(argv[1], "w");

	for (i = 0; i < numlines; i++)
	{
		fwrite(lines[i].str, sizeof(char), lines[i].n, f);
		fwrite(&ent, sizeof(char), 1, f);
	}

	fclose(f);
	free(lines);
	f = fopen(argv[1], "r");

	numlines = rd_lines(f, &lines);
	if(numlines == MEM_ERROR)
	{
		printf("memory error\n");
		return 1;
	}
	printf("\n\n___EDITED_FILE___\n");
	for (int i = 0; i < numlines; i++)
	{
		printf("%s\n", lines[i].str);
	}
	fclose(f);

	return 0;
}