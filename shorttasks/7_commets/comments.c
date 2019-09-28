#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#define BUFFADD 50

#define MEM_ERROR -1

ssize_t fread_line(FILE* f, char** line, int* e)
{
    char* buff = NULL;
    size_t size = 0;
    char* tmp = NULL;
    size_t mem = 0;
    int sym = 0;
    *e = 0;

    while(1)
    {
        sym = fgetc(f);
        if(sym == '\n' || (sym == EOF && (*e = 1))) break;

        if(size == mem)
        {
            tmp = (char*) realloc(buff, (size + BUFFADD) * sizeof(char));
            if(tmp == NULL)
            {
                return MEM_ERROR; 
            }

            buff = tmp;
            mem += BUFFADD;
        }
        buff[size++] = (char)sym;
    }

    tmp = (char*) realloc(buff, (size + 1) * sizeof(char));
    if(tmp == NULL) return MEM_ERROR;

    buff = tmp;
    buff[size] = '\0';
    *line = buff;

    return size;
}

ssize_t rd_lines(FILE* f, char*** lines)
{
    char** buff = NULL, **tmp = NULL;
    size_t size = 0, mem = 0;
    char* line;
    int e;

    while(1)
    {
        if(fread_line(f, &line, &e) < 0) break;

        if(size == mem)
        {
            tmp = (char**) realloc(buff, (size + BUFFADD) * sizeof(char*));
            if(tmp == NULL) return MEM_ERROR;
            buff = tmp;
            mem += BUFFADD;
        }

        buff[size++] = line;
        if(e) break;
    }

    tmp = (char**) realloc(buff, (size + 1) * sizeof(char*));
    if(tmp == NULL) return MEM_ERROR;

    buff = tmp;
    buff[size] = NULL;
    *lines = buff;

    return size;
}

int cmp_lines(const void* p1, const void* p2)
{
    return strcmp(*(char* const*)p1, *(char* const*)p2);
}


ssize_t rm_comments(char** line)
{
    int in_string = 0;
    size_t len;

    char* tmp = NULL;

    for(len = 0; line[0][len] != '\0'; len++)
    {
        line[0][len] == '\"' && (in_string = (in_string ? 0 : 1));

        if(!in_string && line[0][len] == '/' && line[0][len + 1] == '/') break;
    }

    tmp = (char*) realloc(*line, (len + 1) * sizeof(char));
    if(tmp == NULL) return MEM_ERROR;

    tmp[len] = '\0';

    *line = tmp;
    
    return len;
}

int write_file(FILE* f)
{
    char* text = "here is some //text \n\
and then \"some //more\"\n\
and again// text\n\
//comment\n\
testing out// the prog";

    if (fputs(text, f) == EOF) return 1;
    return 0;
}


int main(int argc, char* argv[])
{
    FILE* f;

    char** lines = NULL;

    size_t size, i, len;

	if(argc < 2)
	{
		printf("Not enough arguments\n");
		return 1;
	}

    f = fopen(argv[1], "w");
    write_file(f);
    fclose(f);

	f = fopen(argv[1], "r");

    size = rd_lines(f, &lines);
    
    printf("\n\n___ORIGINAL_FILE___\n");
    for(i = 0; i < size; ++i)
    {
        printf("%s\n", lines[i]);
    }

    qsort(lines, size, sizeof(char*), cmp_lines);

    fclose(f);
    f = fopen(argv[1], "w");

    printf("\n\n___EDITED_FILE___\n");
    for(i = 0; i < size; ++i)
    {
        len = rm_comments(&lines[i]);
        if (len)
        {
            fputs(lines[i], f); 
            fputc('\n', f);

            printf("%s\n", lines[i]);
        }
    }
    printf("\n\n");

    for(i = 0; i < size; ++i)
    {
        free(lines[i]);
    }
    free(lines);

	fclose(f);
	return 0;
}