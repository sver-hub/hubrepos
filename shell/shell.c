#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFADD 20
#define MEM_ERROR -1
#define NEWBUF {NULL, 0, 0}
#define NEWJOB {0, NULL, 0}
#define MAX 10



//BUFFER
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

void freejob(job *jb);

typedef struct process
{
	char *name;
	pid_t pid;
	int status; //0 running in bg | 1 completed | 2 stopped
} process;


process bgprocs[MAX];

//QUEUE
typedef struct QUEUE
{
	int front;
	int rear;
	int num;
	job array[MAX];
} queue;

void addq(queue *q, job a)
{
	if (q->num < MAX)
	{
		if (q->rear == MAX - 1)
			q->rear = -1;

		q->rear++;
		q->array[q->rear] = a;
		q->num++;
	}
	else
		printf("Job can not be run : jobs queue if full\n");
}

job takeq(queue *q)
{
	job a = q->array[q->front];
	q->front++;

	if (q->front == MAX)
		q->front = 0;

	q->num--;
	return a;
}

void resetq(queue *q)
{
	int i;
	for (i = 0; i < q->num; i++)
		freejob(&q->array[i]);
	q->front = 0;
	q->rear = -1;
	q->num = 0;
}

queue jobq;


typedef struct history
{
	int top;
	int num;
	char *commands[MAX];
} history;


struct vars
{
	char *user;
	char *home;
	char *shell;
	int numargs;
	char **args;
	uid_t uid;
	char *pwd;
	pid_t pid;
	char *username;
	int eof;
	history history;
	pid_t curpid;
	char *curname;
};

struct vars V;

//HISTORY
void addhistory(char* s)
{
	if (V.history.top == MAX - 1)
	{
		V.history.top = -1;
	}

	V.history.top++;
	V.history.num++;
	if (V.history.num > MAX) free(V.history.commands[V.history.top]);
	V.history.commands[V.history.top] = s;
}

void freehistory()
{
	int i;
	int t = V.history.num < MAX ? V.history.num : MAX;
	for (i = 0; i < t; i++)
	{
		free(V.history.commands[i]);
	}
}

void err_com()
{
	printf("Invalid input\n");
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
	if (args[1] == NULL || args[2] != NULL)
		return -1;

	if (chdir(args[1])!= 0) 
	{
		printf("No such directory\n");
		return -1;
	}

	free(V.pwd);
	V.pwd = getcwd(NULL, 1);
	//realpath(args[0], NULL);

	return 0;
}

int sh_pwd(char **args)
{
	if (args[1] != NULL)
		return -1;
	printf("%s\n", V.pwd);
	return 0;
}

int sh_jobs(char **args)
{
	int i;

	if (args[1] != NULL) return -1;

	for (i = 0; i < MAX; i++)
	{
		if (bgprocs[i].name != NULL)
		{
			printf("[%d]\t", i + 1);
			if (bgprocs[i].status == 0) printf("Running");
			else if (bgprocs[i].status == 1) printf("Done");
			else printf("Stopped");
			printf("\t\t%s\n", bgprocs[i].name);

			if (bgprocs[i].status == 1)
			{
				free(bgprocs[i].name);
				bgprocs[i].name = NULL;
			}
		}
	}
	return 0;
}

int sh_fg(char **args)
{
	int n;
	int status;
	pid_t wpid;

	if (args[1] == NULL || args[2] != NULL)
		return -1;

	n = atoi(args[1]);
	if (n < 1) 
	{
		printf("Illigal argument\n");
		return -1;
	}

	if (n > MAX || bgprocs[n - 1].name == NULL || bgprocs[n - 1].status == 1)
	{
		printf("No suspended job with this index\n");
		return -1;
	}

	V.curpid = bgprocs[n - 1].pid;
	printf("%s\n", bgprocs[n - 1].name);

	free(bgprocs[n - 1].name);
	bgprocs[n - 1].name = NULL;

	if (bgprocs[n - 1].status == 2)
		kill(bgprocs[n - 1].pid, SIGCONT);

	do
	{
		wpid = waitpid(bgprocs[n - 1].pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));


	V.curpid = V.pid;

	return 0;
}

int sh_bg(char **args)
{
	int n;

	if (args[1] == NULL || args[2] != NULL)
		return -1;

	n = atoi(args[1]);
	if (n < 1) 
	{
		printf("Illigal argument\n");
		return -1;
	}

	if (n > MAX || bgprocs[n - 1].name == NULL || bgprocs[n - 1].status != 2)
	{
		printf("No suspended job with this index\n");
		return -1;
	}

	bgprocs[n - 1].status = 0;
	printf("Running %s in background\n", bgprocs[n - 1].name);

	kill(bgprocs[n - 1].pid, SIGCONT);
	
	return 0;
}

int sh_exit(char **args)
{
	if (args[1] != NULL)
		return -1;

	onexit();
	exit(0);
	return 0;
}

int sh_history(char **args)
{
	if (args[1] != NULL)
		return -1;

	int i, j, t, n;
	j = V.history.num > MAX ? (V.history.top + 1) % MAX : 0;
	t = V.history.num < MAX ? V.history.num : MAX;
	n = V.history.num - MAX;
	if (n < 0) n = 0;
	for (i = 0; i < t; i++)
	{
		if (j == MAX) j = 0;
		printf("%d %s\n", 1 + n + i, V.history.commands[j]);
		j++;
	}

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

int readline()
{
	buffer buf = NEWBUF;

	buffer subst = NEWBUF;
	int cmnum;

	char c = fgetc(stdin);

	while (c != '\n')
	{
		if (c == '!' && (buf.len == 0 || buf.chars[buf.len - 1] == ' '))
		{
			c = fgetc(stdin);
			while (c != '\n' && c != ' ' && c != ';')
			{
				append(&subst, &c, 1);
				c = fgetc(stdin);
			}

			endbuf(&subst);
			cmnum = atoi(subst.chars);

			if (cmnum == 0)
			{
				append(&buf, subst.chars, subst.len);
			}
			else if (cmnum > V.history.num || cmnum <= V.history.num - MAX)
			{
				printf("Can not substitute command : command is out of reach\n");
				return -1;
			}
			else
			{
				append(&buf, V.history.commands[cmnum % MAX - 1], 169);
			}

			free(subst.chars);
			resetbuf(&subst);
		}
		else
		{
			append(&buf, &c, 1);
			c = fgetc(stdin);
		}
		
	}
	
	endbuf(&buf);

	addhistory(buf.chars);
	
	return 0;
}

//todo error handling
int splitcom(char ***result, int id)
{
	int num = 0;
	char** coms = NULL;
	int i = 0;
	char *line = V.history.commands[id];
	
	buffer buf = NEWBUF;

	while (line[i] != '\0')
	{
		if (line[i] == ';')
		{
			coms = (char**)realloc(coms, sizeof(char*)*(num + 1));
			if (coms == NULL) return MEM_ERROR;

			endbuf(&buf);
			coms[num++] = buf.chars;

			resetbuf(&buf);
		}
		else
			append(&buf, &line[i], 1);

		i++;
	}

	if (buf.len > 0)
	{
		coms = (char**)realloc(coms, sizeof(char*)*(num + 1));
		if (coms == NULL) return MEM_ERROR;

		endbuf(&buf);
		coms[num++] = buf.chars;

		*result = coms;
	}

	return num;
}

//todo error handling
int parsecom(char *line, char ***tokens)
{
	char **args = NULL;
	int numargs = 0;
	int memargs = 0;
	
	buffer buf = NEWBUF;
	buffer subst = NEWBUF;
	char c;
	int i = 0;
	int quotes1 = 0;
	int quotes2 = 0;

	while (1)
	{
		c = line[i++];

		if ((!quotes1 && !quotes2 && (c == ' ' || c == '#')) || c == '\0' || c == EOF)
		{
			if (c == ' ' && buf.len == 0)
				continue;

			if (buf.len > 0)
			{
				endbuf(&buf);

				if (++numargs > memargs)
				{
					args = (char**)realloc(args, sizeof(char*)*(memargs + BUFFADD));
					if (args == NULL) return MEM_ERROR;
					memargs += BUFFADD;
				}

				args[numargs - 1] = buf.chars;

				resetbuf(&buf);
			}

			if (c == '\0' || c == '#' || c == EOF)
			{
				args = (char**)realloc(args, sizeof(char*)*(numargs));
				if (args == NULL) return MEM_ERROR;

				*tokens = args;

				if (c == '#')
				{
					while (c != '\0' && c != EOF)
						c = line[i++];
				}

				if (c == EOF) V.eof = 1;

				return numargs;
			}
		}
		else if (c == '\'')
		{
			if (quotes1 && quotes2 && quotes1 > quotes2)
			{
				err_com();
				return -1;
			}
			quotes1 = quotes1 ? 0 : 1;
			if (quotes2) quotes2++;
		}
		else if (c == '\"')
		{
			if (quotes2 && quotes1 && quotes2 > quotes1)
			{
				err_com();
				return -1;
			}
			quotes2 = quotes2 ? 0 : 1;
			if (quotes1) quotes1++;
		}
		else if (c == '\\')
		{
			c = line[i++];
			if (c == 'n') c = '\n';
			else if (c == 'r') c = '\r';
			else if (c == '\\') c = '\\';
			else if (c == 't') c = '\t';
			else if (c == '\'') c = '\'';
			else if (c == '\"') c = '\"';
		}
		else if (c == '$' && (quotes2 && (!quotes1 || (quotes2 < quotes1))))
		{
			c = line[i++];

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
				while ((c = line[i++]) != '}')
				{
					append(&subst, &c, 1);
				}
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
				resetbuf(&subst);
			}
			else
			{
				append(&buf, "$", 1);
				append(&buf, &c, 1);
			}
		}
		else
		{
			if (quotes1) quotes1++;
			if (quotes2) quotes2++;

			append(&buf, &c, 1);
		}
	}
}

//maybe TODO exit status
int parse_job(char **tokens, int numtokens)
{
	int j;
	job jb;

	program *progs = NULL;
	int iprog = 0;

	char **args = NULL;
	int iarg = 0;
	int memarg = 0;


	progs = malloc(sizeof(program));
	if (progs == NULL) return MEM_ERROR;
	progs[0].input_file = NULL;
	progs[0].output_file = NULL;
	progs[0].output_type = 0;
	
	for (j = 0; j < numtokens; j++)
	{
		if (!strcmp(tokens[j], "|"))
		{
			args = (char**)realloc(args, sizeof(char*)*(iarg + 1));
			if (args == NULL) return MEM_ERROR;

			args[iarg] = NULL;
			progs[iprog].arguments = args;
			progs[iprog].number_of_arguments = iarg;

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
		else if (!strcmp(tokens[j], "&"))
		{
			if (j == numtokens - 1)
				jb.background = 1;
			else
				return -1;
		}
		else
		{
			if (iarg == 0)
				progs[iprog].name = tokens[j];
			
			if (memarg < iarg + 1)
			{
				memarg += BUFFADD;
				args = (char**)realloc(args, sizeof(char*)*memarg);
				if (args == NULL) return MEM_ERROR;
			}

			args[iarg] = tokens[j];
			iarg++;
		}
	}

	args = (char**)realloc(args, sizeof(char*)*(iarg + 1));
	if (args == NULL) return MEM_ERROR;

	args[iarg] = NULL;
	progs[iprog].arguments = args;
	progs[iprog].number_of_arguments = iarg;

	jb.programs = progs;
	jb.number_of_programs = iprog + 1;

	addq(&jobq, jb);

	return 0;
}

int get_jobs()
{
	char **splitted = NULL;
	int sp;
	char **tokens = NULL;
	int tk;
	int i;

	sp = splitcom(&splitted, V.history.top);

	if (sp == 0) return 0;

	for (i = 0; i < sp; i++)
	{
		tk = parsecom(splitted[i], &tokens);
		parse_job(tokens, tk);
		free(splitted[i]);
	}

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

int execute(job jb)
{
	int i;
	int iprog;
	pid_t wpid;
	int status;
	pid_t pid;
	int p[2];
	int prevrd;
	int filefd;

	buffer curname = NEWBUF;

	for (i = 0; i < 6; i++)
	{
		if (!strcmp(jb.programs[0].name, sh_names[i]))
		{
			return (*sh_funcs[i])(jb.programs[0].arguments);
		}
	}

	for (iprog = 0; iprog < jb.number_of_programs; iprog++)
	{
		if (jb.number_of_programs > 0 && iprog < jb.number_of_programs - 1)
			if (pipe(p) == -1) return -1;

		pid = fork();

		if (pid == -1) return -1;
		if (pid == 0) 
		{
			if (iprog > 0)
			{
				//input from previous program
				dup2(prevrd, 0);
				close(prevrd);
			}

			if (iprog < jb.number_of_programs - 1)
			{
				//output to next program
				dup2(p[1], 1);
				close(p[0]);
				close(p[1]);
			}
			
			//file input
			if (jb.programs[iprog].input_file != NULL)
			{
				if (access(jb.programs[iprog].input_file, F_OK) == -1)
				{
					printf("Input_file does not exist\n");
					_exit(1);
				}

				filefd = open(jb.programs[iprog].input_file, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(filefd, 0);
				close(filefd);
			}	

			//file output
			if (jb.programs[iprog].output_file != NULL)
			{
				if (jb.programs[iprog].output_type == 1)
					filefd = open(jb.programs[iprog].output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
				else
					filefd = open(jb.programs[iprog].output_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(filefd, 1);
				close(filefd);
			}

			if (!strcmp(jb.programs[iprog].name, "history"))
				sh_history(jb.programs[iprog].arguments);
			else
			{
				execvp(jb.programs[iprog].name, jb.programs[iprog].arguments);
				printf("Failed\n");
			}
			_exit(1);
		}

		if (iprog < jb.number_of_programs - 1) close(p[1]);
		if (iprog > 0) close(prevrd);

		prevrd = p[0];

		if (iprog == 0) append(&curname, jb.programs[iprog].name, 169);
		else
		{
			append(&curname, " | ", 3);
			append(&curname, jb.programs[iprog].name, 169);
		}
	}

	V.curpid = pid;
	endbuf(&curname);
	if (V.curname != NULL) free(V.curname);
	V.curname = curname.chars;

	if (jb.background == 0)
	{
		do
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));
	}
	else
	{
		for (i = 0; i < MAX; i++)
		{
			if (bgprocs[i].name == NULL)
			{
				bgprocs[i].name = V.curname;
				bgprocs[i].pid = V.curpid;
				bgprocs[i].status = 0;

				printf("[%d] %d\n", i + 1, V.curpid);
				break;
			}
		}

		V.curname = NULL;
	}

	V.curpid = V.pid;

	return 0;
}

void freejob(job *jb)
{
	int i, j;

	for (i = 0; i < jb->number_of_programs; i++)
	{
		free(jb->programs[i].name);
		for (j = 1; j < jb->programs[i].number_of_arguments; j++)
		{
			free(jb->programs[i].arguments[j]);
		}
		free(jb->programs[i].arguments);
		if (jb->programs[i].input_file != NULL) free(jb->programs[i].input_file);
		if (jb->programs[i].output_file != NULL) free(jb->programs[i].output_file);
	}
}

/* MAIN */
int init(int argc, char **argv)
{
	int i;
	struct passwd *pwd;

	V.user = getenv("USER");
	V.home = getenv("HOME");
	V.pwd = getcwd(NULL, 1);
	V.pid = getpid();
	V.uid = getuid();
	V.curpid = V.pid;
	V.curname = NULL;

	pwd = getpwuid(V.uid);
	V.username = pwd->pw_name;

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

	V.history.top = -1;
	V.history.num = 0;

	for (i = 0; i < MAX; i++)
	{
		bgprocs[i].name = NULL;
	}

	resetq(&jobq);

	return 0;
}

int onexit()
{
	free(V.pwd);
	free(V.shell);
	freehistory();
	
	return 0;
}

void inthndlr(int sig)
{
	sig += 0;
	printf("\n%d\n", getpid());
	if (V.curpid != V.pid)
	{
		kill(V.curpid, SIGTERM);
	}
}

void stophndlr(int sig)
{
	sig += 0;
	int i;
	buffer buf = NEWBUF;

	printf("\n");
	if (V.curpid != V.pid)
	{
		kill(V.curpid, SIGTSTP);
		for (i = 0; i < MAX; i++)
		{
			if (bgprocs[i].name == NULL)
			{
				append(&buf, V.curname, 169);
				endbuf(&buf);
				bgprocs[i].name = buf.chars;
				bgprocs[i].pid = V.curpid;
				bgprocs[i].status = 2;
				printf("[%d]\tStopped\t%s\n", i + 1, V.curname);
				break;
			}
		}
	}
}

void chldhndlr(int sig)
{
	sig += 0;
	int pid;
	int status;
	int i;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		for (i = 0; i < MAX; i++)
		{
			if (bgprocs[i].name != NULL && bgprocs[i].pid == pid) 
				bgprocs[i].status = 1;
		}
	}
}

int main(int argc, char** argv)
{
	
	job jb;

	init(argc, argv);

	signal(SIGINT, inthndlr);
	signal(SIGTSTP, stophndlr);
	signal(SIGCHLD, chldhndlr);
	
	while (!V.eof)
	{
		printf("%s$ ", V.username);

		readline();

		get_jobs();


		while (jobq.rear >= jobq.front)
		{
			
			jb = takeq(&jobq);
	
			execute(jb);
			freejob(&jb);
			
		}
		resetq(&jobq);
	}

	onexit();
	

	
}
