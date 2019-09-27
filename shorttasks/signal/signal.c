#include <stdio.h>
#include <signal.h>

volatile int g_max = -1;

void sigint_handler(int sig, siginfo_t* info, void* uncontext)
{
	volatile static int i = 0;

	if(g_max <= 0) return;

	if(++i != g_max)
	{
		printf("%d\t%i\n", i,(int)info->si_pid);
	}
	else
	{
		printf("bye\n");
		exit(0);
	}
}

int main(int argc, char** argv) 
{
	struct sigaction sa;
	sa.sa_sigaction = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_restore = NULL;
	sigaction(SIGINT, &sa, NULL);
	g_max = atoi(argv[1]);
	if (g_max < 1) 
	{
		printf("wrong arg");
		return 0;
	}
	while(1)
		pause();
	return 0;
}

