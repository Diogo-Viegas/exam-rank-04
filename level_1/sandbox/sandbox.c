#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


static pid_t g_pid = -1;

void timeout_handler(int sig)
{
	(void)sig;
	if(g_pid > 0)
		kill(g_pid,SIGKILL);
}
int sandbox(void (*f)(void),unsigned int timeout, bool verbose)
{
	pid_t pid;
	int status;
	struct sigaction sa;
	
	pid = fork();
	if(pid < 0)
		return (-1);
	if(pid == 0)
	{
		f();
		exit(0);
	}

	g_pid = pid;

	sa.sa_handler = timeout_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGALRM,&sa,NULL) < 0)
		return (-1);
	alarm(timeout);
	while(waitpid(pid,&status,0) == -1)
	{
		if(errno != EINTR)
			return(-1);
	}
	alarm(0);
	if(WIFEXITED(status))
	{
		int code = WEXITSTATUS(status);
		if(code == 0)
		{
			if(verbose)
				printf("Nice function!\n");
			return (1);
		}
		if(verbose)
			printf("Bad function: exited with code %d\n",code);
		return (0);
	}
	if(WIFSIGNALED(status))
	{
		int sig = WTERMSIG(status);
		if(verbose)
		{
			if(sig == SIGKILL)
				printf("Bad function: timed out after %u seconds\n",timeout);
			else
				printf("Bad function: %s\n",strsignal(sig));
		}
		return (0);
	}
	return (0);
}
