#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define NEWBUF {NULL, 0, 0}
#define BUFFADD 20
#define MEM_ERROR -1

typedef struct buffer
{
	char *chars;
	int len;
	int mem;
} buffer;

int append(buffer *buf, char* s, int l)
{
	int i;
	
	for (i = 0; i < l; i++)
	{
		buf->len++;
		if (buf->len > buf->mem)
		{
			buf->mem += BUFFADD;
			buf->chars = (char*)realloc(buf->chars, buf->mem);

			if (buf->chars == NULL) return MEM_ERROR;
		}

		buf->chars[buf->len - 1] = s[i];
	}

	return i;
}

int endbuf(buffer *buf)
{
	char *tmp = NULL;
	tmp = (char*)realloc(buf->chars, (buf->len + 1)*sizeof(char));
	if (tmp == NULL) return MEM_ERROR;

	tmp[buf->len] = '\0';
	buf->chars = tmp;
	buf->mem = buf->len + 1;

	return 0;
}

void resetbuf(buffer *buf)
{
	buf->chars = NULL;
	buf->len = 0;
	buf->mem = 0;
}

typedef struct
{
	char *chars;
	int len;
} str;

str* lines = NULL;
int numlines = 0;
int lmem = 0;

int readlines()
{
	char c;
	buffer buf = NEWBUF;

	c = getc(stdin);

	while (c != EOF)
	{
		if (c == '\n')
		{
			endbuf(&buf);

			if (numlines + 1 > lmem)
			{
				lmem += BUFFADD;
				lines = realloc(lines, lmem * sizeof(str));
				if (lines == NULL) return MEM_ERROR;
			}

			lines[numlines].chars = buf.chars;
			lines[numlines++].len = buf.len;
			resetbuf(&buf);
		}
		else append(&buf, &c, 1);

		
		c = getc(stdin);
	}

	if (buf.len > 0)
	{
		endbuf(&buf);
		lines[numlines].chars = buf.chars;
		lines[numlines++].len = buf.len;
	}
	
	return 0;
}

enum state
{
    READY,
    MUL,
    FINISH,
    UNKNOWN,
    ERR
};


typedef struct
{
    enum state state;
    int index;
    pthread_mutex_t mut;
    pthread_cond_t cond;
} Thread_params;

Thread_params *thread_params = NULL;

int Max = 0;
char *Word = NULL;
int Count = 0;

int eof = 0;
int start;
int curr;

void *body(void *args)
{  
    Thread_params *params = (Thread_params *)args;
    str line;

    while (1)
    {
        pthread_mutex_lock(&params->mut);

        if (params->state == FINISH)
        {
            pthread_mutex_unlock(&params->mut);
            return NULL;
        }

        params->state = READY;

        pthread_cond_wait(&params->cond, &params->mut);

        if (params->state != MUL)
        {
            pthread_mutex_unlock(&params->mut);
            continue;
        }

        line = lines[params->index];

        pthread_mutex_unlock(&params->mut);

        {	
        	int i;
            int max = 0;
            char *word = NULL;
            int count = 0;
            buffer buf = NEWBUF;

            for (i = 0; i <= line.len; i++)
            {
                if (i == line.len || (line.chars[i] == ' ' && buf.len > 0))
                {
                	endbuf(&buf);

                	if (max < buf.len)
                	{	
                		max = buf.len;
                		if (word != NULL) free(word);
                		word = buf.chars;
                		count = 1;
                	}
                	else if (max == buf.len && !strcmp(word, buf.chars))
                	{
                		count++;
                	}

                	resetbuf(&buf);
                }
                else append(&buf, &line.chars[i], 1);
            }

            if (Max < max)
            {
            	Max = max;
            	if (Word != NULL) free(Word);
            	Word = word;
            	Count = 1;
            }
            else if (Max == max && !strcmp(Word, word))
            {
            	Count += count;
            	free(word);
            }
            else
            {
            	free(word);
            }
        }
        
    }

    return NULL;
}

void *readerbody(void *args)
{
	readlines();
	eof = 1;
	return NULL;
}


int main(int argc,char **argv)
{
    int num_threads = 0;
    int i;
    int finish_counter = 0;
    int current_index = 0;

    pthread_t *threads = NULL;
    pthread_t reader;


    if (argc < 2)
    {
        fprintf(stderr,"num_threads in args\n");
        return 1;
    }

    num_threads = atoi(argv[1]);
    if (num_threads <= 0)
    {
        fprintf(stderr, "We see %d threads\n But it too little", num_threads);
        return 1;
    }

    threads = (pthread_t *)malloc(sizeof(pthread_t)*num_threads);
    if (threads == NULL)
    {
        return 2;
    }

    thread_params = (Thread_params *)malloc(sizeof(Thread_params)*num_threads);
    if (thread_params == NULL)
    {
        return 2;
    }

    if (pthread_create(&reader, NULL, readerbody, NULL) == -1)
    {
    	return 3;
    }

    start = clock();


    while (!eof)
    {    
    	curr = clock();

    	if ((float)(curr - start) /CLOCKS_PER_SEC >= 10)
    	{
    		finish_counter = 0;
	    	current_index--;
	    	if (current_index < 0) current_index = 0;
	    	printf("checking...\n");
		    for (i = 0; i < num_threads; i++)
		    {
		        if (pthread_mutex_init(&thread_params[i].mut, NULL) == -1)
		        {
		            return 3;
		        }

		        if (pthread_cond_init(&thread_params[i].cond, NULL) == -1)
		        {
		            return 3;
		        }

		        thread_params[i].index = 0;
		        thread_params[i].state = UNKNOWN;

		        if (pthread_create(&threads[i], NULL, body, &thread_params[i]) == -1)
		        {
		            return 3;
		        }
		    }

		    while (1)
		    {
		        if (current_index == numlines)
		        {
		            break;
		        }

		        for (i = 0; i < num_threads; i++)
		        {
		        	
		            pthread_mutex_lock(&thread_params[i].mut);

		            if (thread_params[i].state == READY)
		            {
		                if (current_index == numlines)
		                {
		                    thread_params[i].state = FINISH;
		                    finish_counter++;
		                }
		                else
		                {
		                    thread_params[i].index = current_index;
		                    thread_params[i].state = MUL;
		                    current_index++;
		                }

		                pthread_cond_signal(&thread_params[i].cond);
		                
		                
		            }
		            pthread_mutex_unlock(&thread_params[i].mut);
		                
		        } /* end for */
		    } /* end  while */

		    while (finish_counter < num_threads)
		    {
		        for (i = 0; i < num_threads; i++)
		        {
		            pthread_mutex_lock(&thread_params[i].mut);
		            if (thread_params[i].state == READY)
		            {
		                thread_params[i].state = FINISH;
		                pthread_cond_signal(&thread_params[i].cond);
		                finish_counter++;
		            }
		            pthread_mutex_unlock(&thread_params[i].mut);
		        }
		    }
		    start = clock();
		}
	}

	pthread_join(reader, NULL);


    for (i = 0; i < num_threads; i++)
    {
            pthread_join(threads[i], NULL);
            pthread_mutex_destroy(&thread_params[i].mut);
            pthread_cond_destroy(&thread_params[i].cond);
    }

    free(threads);
    free(thread_params);

    printf("Longest word - \t%s\nwith length of \t%d\nencounteres \t%d\n", Word, Max, Count);

    return 0;
}
