#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main()
{
	pid_t pid, pidpr1, pidpr2, ppid;
	int status;
	int p[2];

	pipe(p);
	pid = fork();

	if (pid == 0)
	{
		pidpr1 = getpid();
		write(p[1], &pidpr1, sizeof(int));
		close(p[0]);
		close(p[1]);
		_exit(0);
	}

	close(p[1]);

	pid = fork();

	if (pid == 0)
	{
		read(p[0], &pidpr1, sizeof(int));
		close(p[0]);

		pidpr2 = getpid();
		ppid = getppid();

		printf("process 1 pid = %d\nprocess 2 pid = %d\nparent pid = %d\n", pidpr1, pidpr2, ppid);

		if (pidpr1 % 2 == 0)
		{
			execlp("ls", "ls", "-l", NULL);
		}
		else 
		{
			execlp("df", "df", "-h", NULL);
		}

		printf("error\n");
		_exit(1);
	}

	close(p[0]);
	
	do
	{
		waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));
}
