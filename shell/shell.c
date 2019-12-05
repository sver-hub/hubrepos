#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>

#define BUFFADD 20
#define MEM_ERROR -1
#define NEWBUF {NULL, 0, 0}

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
		if (l == 169 && s[i] == '\0')
			break;

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


typedef struct program
{
	char *name;
	int number_of_arguments;
	char **arguments;
	char *input_file, *output_file; //null if std
	int output_type; // 1 rewrite 2 append
} program;

typedef struct job
{
	int background;
	program* programs;
	int number_of_programs;
} job;

struct vars
{
	char *user;
	char *home;
	char *shell;
	int numargs;
	char **args;
	int uid;
	char *pwd;
	int pid;	
};

struct vars V;

void err_com()
{
	printf("invalid input\n");
}


/* COMMANDS */
int sh_cd(char **args);
int sh_pwd(char **args);
int sh_jobs(char **args);
int sh_fg(char **args);
int sh_bg(char **args);
int sh_exit(char **args);
int sh_history(char **args);
int onexit();

char *sh_names[] = {
  "cd",
  "pwd",
  "jobs",
  "fg",
  "bg",
  "exit",
  "history"
};

int (*sh_funcs[]) (char **) = {
  &sh_cd,
  &sh_pwd,
  &sh_jobs,
  &sh_fg,
  &sh_bg,
  &sh_exit,
  &sh_history
};

int sh_cd(char **args)
{
	if (args[0] == NULL || args[1] != NULL)
		return -1;

	if (chdir(args[0])!= 0) 
	{
		printf("no such directory\n");
		return -1;
	}

	free(V.pwd);
	V.pwd = realpath(args[0], NULL);

	return 0;
}

int sh_pwd(char **args)
{
	if (args[0] != NULL)
		return -1;
	printf("%s\n", V.pwd);
	return 0;
}

int sh_jobs(char **args)
{
	printf("%s\n", args[0]);
	return 0;
}

int sh_fg(char **args)
{
	printf("%s\n", args[0]);
	return 0;
}

int sh_bg(char **args)
{
	printf("%s\n", args[0]);
	return 0;
}

int sh_exit(char **args)
{
	if (args[0] != NULL)
		return -1;
	
	onexit();
	exit(0);
	return 0;
}

int sh_history(char **args)
{
	printf("%s\n", args[0]);
	return 0;
}


/* READ COMMANDS */
char* itoa(int n)
{
	char *res;
	int length = 1;
	int i;
	int del = 10;
	char swap;

	while (n / del > 0) 
	{
		length++;
		del *= 10;
	}

	res = malloc(length + 1);

	del = 1;
	for (i = 0; i < length; i++)
	{
		res[i] = ((n / del) % 10) + '0';
		del *= 10;
	}

	for (i = 0; i < length / 2; i++)
	{
		swap = res[length - 1 - i];
		res[length - 1 - i] = res[i];
		res[i] = swap;
	}

	res[length] = '\0';

	return res;
}

int read_command(char ***tokens)
{
	char **args = NULL;
	int numargs = 0;
	int memargs = 0;
	
	buffer buf = NEWBUF;
	buffer subst = NEWBUF;
	char c;
	int quotes1 = 0;
	int quotes2 = 0;

	while (1)
	{
		c = fgetc(stdin);

		if ((!quotes1 && !quotes2 && (c == ' ' || c == '#' || c == ';')) || c == '\n')
		{
			if (c == ' ' && buf.len == 0)
				continue;

			if (buf.len > 0)
			{
				buf.chars = (char*)realloc(buf.chars, buf.len + 1);
				if (buf.chars == NULL) return MEM_ERROR;
				append(&buf, "\0", 1);

				if (++numargs > memargs)
				{
					args = (char**)realloc(args, sizeof(char*)*(memargs + BUFFADD));
					if (args == NULL) return MEM_ERROR;
					memargs += BUFFADD;
				}

				args[numargs - 1] = buf.chars;
			}

			if (c == '\n' || c == '#' || c == ';')
			{
				*tokens = args;
				return numargs;
			}

			buf.chars = NULL;
			buf.len = 0;
			buf.mem = 0;
		}
		else
		{
			if (c == '\'')
			{
				if (quotes1 && quotes1 > quotes2)
				{
					free(buf.chars);
					return -1;
				}
				quotes1 = quotes1 ? 0 : 1;
				if (quotes2) quotes2++;
				continue;
			}
			else if (c == '\"')
			{
				if (quotes2 && quotes2 > quotes1)
				{
					free(buf.chars);
					return -1;
				}
				quotes2 = quotes2 ? 0 : 1;
				if (quotes1) quotes1++;
				continue;
			}

			if (c == '\\')
			{
				c = fgetc(stdin);
				if (c == 'n') c = '\n';
				else if (c == 'r') c = '\r';
				else if (c == '\\') c = '\\';
				else if (c == 't') c = '\t';
				else if (c == '\'') c = '\'';
				else if (c == '\"') c = '\"';
				else continue;
			}

			if (c == '$' && (!quotes1 || (quotes2 && quotes2 < quotes1)))
			{
				c = fgetc(stdin);

				if (c > '0' && c <= '9')
				{
					append(&buf, V.args[c - '0' - 1], 169);
				}
				else if (c == '#')
				{
					c = V.numargs + '0';
					append(&buf, &c, 1);
				}
				else if (c == '?')
				{

				}
				else if (c == '{')
				{
					while ((c = fgetc(stdin)) != '}')
						append(&subst, &c, 1);
					append(&subst, "\0", 1);

					if (!strcmp(subst.chars, "HOME"))
					{
						append(&buf, V.home, 169);
					}
					else if (!strcmp(subst.chars, "USER"))
					{
						append(&buf, V.user, 169);
					}
					else if (!strcmp(subst.chars, "SHELL"))
					{
						append(&buf, V.shell, 169);
					}
					else if (!strcmp(subst.chars, "PWD"))
					{
						append(&buf, V.pwd, 169);
					}
					else if (!strcmp(subst.chars, "UID"))
					{
						char *num = itoa(V.uid);
						append(&buf, num, 169);
						free(num);
					}
					else if (!strcmp(subst.chars, "PID"))
					{
						char *num = itoa(V.pid);
						append(&buf, num, 169);
						free(num);
					}

					free(subst.chars);
					subst.chars = NULL;
					subst.len = 0;
					subst.mem = 0;
				}
				continue;
			}

			if (quotes1) quotes1++;
			if (quotes2) quotes2++;

			append(&buf, &c, 1);
		}
	}
}

int get_job(job *jb)
{
	int j;

	char** tokens = NULL;
	int numtokens;

	program *progs = NULL;
	int iprog = 0;

	char **args = NULL;
	int iarg = 0;
	int memarg = 0;

	jb->background = 0;

	progs = malloc(sizeof(program));
	if (progs == NULL) return MEM_ERROR;
	progs[0].input_file = NULL;
	progs[0].output_file = NULL;
	progs[0].output_type = 0;

	numtokens = read_command(&tokens);
	
	for (j = 0; j < numtokens; j++)
	{
		if (!strcmp(tokens[j], "|"))
		{
			args = (char**)realloc(args, sizeof(char*)*(iarg+1));
			if (args == NULL) return MEM_ERROR;

			args[iarg] = NULL;
			progs[iprog].arguments = args;
			progs[iprog].number_of_arguments = iarg - 1;

			iprog++;
			progs = (program*)realloc(progs, sizeof(program)*(iprog + 1));
			if (progs == NULL) return MEM_ERROR;
			progs[iprog].input_file = NULL;
			progs[iprog].output_file = NULL;
			progs[iprog].output_type = 0;

			args = NULL;
			iarg = 0;
			memarg = 0;
		}
		else if (!strcmp(tokens[j], "<"))
		{
			if (++j == numtokens)
			{
				err_com();
				return -1;
			}
			progs[iprog].input_file = tokens[j];
		}
		else if (!strcmp(tokens[j], ">") || !strcmp(tokens[j], ">>"))
		{
			if (++j == numtokens)
			{
				err_com();
				return -1;
			}
			progs[iprog].output_file = tokens[j];
			progs[iprog].output_type = !strcmp(tokens[j - 1], ">") ? 1 : 2;
		}
		else if (j == numtokens - 1 && !strcmp(tokens[j], "&"))
			jb->background = 1;
		else
		{
			if (iarg == 0)
				progs[iprog].name = tokens[j];
			else
			{
				if (memarg < iarg)
				{
					memarg += BUFFADD;
					args = (char**)realloc(args, sizeof(char*)*memarg);
					if (args == NULL) return MEM_ERROR;
				}
				args[iarg - 1] = tokens[j];
			}

			iarg++;
		}
	}
	args = (char**)realloc(args, sizeof(char*)*(iarg + 1));
	if (args == NULL) return MEM_ERROR;

	args[iarg] = NULL;
	progs[iprog].arguments = args;
	progs[iprog].number_of_arguments = iarg - 1;

	jb->programs = progs;
	jb->number_of_programs = iprog + 1;


	return 0;
}

void print_job(job jb)
{
	int i, j;

	for (i = 0; i < jb.number_of_programs; i++)
		{
			printf("program %d:\n\tname : %s\n\tnum of agrs : %d", i + 1, jb.programs[i].name, jb.programs[i].number_of_arguments);
			for (j = 0; j < jb.programs[i].number_of_arguments; j++)
				printf("\n\targument %d : %s", j + 1, jb.programs[i].arguments[j]);
			printf("\n\tinput file : %s\n\toutput file : %s\n\toutput type : %d\n", jb.programs[i].input_file == NULL ? "NULL" : jb.programs[i].input_file,
				jb.programs[i].output_file == NULL ? "NULL" : jb.programs[i].output_file, jb.programs[i].output_type);
		}
		printf("background : %d\n", jb.background);
}


/* MAIN */
int init(int argc, char **argv)
{
	int i;

	V.user = getenv("USER");
	V.home = getenv("HOME");
	V.pwd = getcwd(NULL, 1);
	V.pid = getpid();
	V.uid = getuid();

	V.shell = realpath(argv[0], NULL);

	if (argc > 1)
	{
		V.numargs = argc - 1;
		V.args = (char**)malloc(V.numargs*sizeof(char*));
		for (i = 0; i < V.numargs; i++)
		{
			V.args[i] = argv[i + 1];
		}
	}
	else 
	{
		V.numargs = 0;
		V.args = NULL;
	}

	return 0;
}

int onexit()
{
	free(V.pwd);
	free(V.shell);
	
	return 0;
}

int main(int argc, char** argv)
{
	
	job jb;

	init(argc, argv);
	printf("%d\n", V.numargs);
	get_job(&jb);

	print_job(jb);

	onexit();
	

	
}
