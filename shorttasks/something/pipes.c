#include <>

int searcher(void)
{
	pid_t pids[3];
	int pipes[2][2];
	if(pipe(pipes[0]) == -1)
	{
		perror("pipe 1");
		return 1;
	}
	pids[0] = fork();
	if (pids[0] == -1) return 1;
	if (pids[0] == 0)
	{
		dup2(pipes[0][1], 1);
		close(pipes[0][1]);
		close(pipes[0][0]);
		execlp("ls", "ls", "-R", NULL);
		perror("exec 1 failed");
		exit(1);
	}
	close(pipes[0][1]);
	if (pipe(pipes[1]) == -1) return 1;
	pids[1] = fork();
	if (pids[1] == -1) return 1;
	if (pids[1] == 0) 
	{
		dup2(pipes[0][0], 0);
		close(pipes[0][0]);
		dup2(pipes[1][1], 1);
		close(pipes[1][1]);
		close(pipes[1][0])
		execlp("grep", "grep", ".txt", NULL);
		perror("exec 2 failed");
		exit(1);
	}
	close(pipes[0][0]);
	close(pipes[1][1]);
	pids[2] = fork();
	if (pids[2] == -1) return 1;
	if (pids[2] == 0)
	{
		dup2(pipes[1][0], 0);
		close(pipes[1][0]);
		execlp("wc", "wc", NULL);
		perror("exec 3 failed");
		exit(1);
	}
	
}