#include <stdio.h>
#define ERROR_OPEN 21

int prog(const char* prog, const char* path_f1, const char* path_f2)
{
	int fd_in, fd_out, status;
	pid_t pid;

	fd_in = open(path_f1, O_RDONLY, 0);
	fd_out = open(path_f2, O_APPEND | O_CREAT, 0);
	if (fd_in == -1 || fd_out == -1) 
	{
		printf("failed openning\n");
		return ERROR_OPEN;
	}
	pid = fork();
	if (pid == -1) return 1;
	if (pid == 0)
	{
		dup2(fd_out, 1);
		dup2(fd_in, 0);
		close(fd_in);
		close(fd_out);
		execlp(prog, prog, NULL);
		perror("prog");
		exit(1);
	}
	close(fd_in);
	close(fd_out);
	wait(&status);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) fprintf(stderr, "OK");
	else fprintf(stderr, "NOT OK");
	return 0
}