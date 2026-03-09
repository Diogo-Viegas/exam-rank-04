#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int picoshell(char **cmds[])
{
    int fd[2];
    int fd_in = 0;
    pid_t pid;
    int i = 0;
    while(cmds[i])
    {
        if(cmds[i + 1] && pipe(fd) == -1)   
            return (1);
        pid = fork();
        if(pid == -1)
            return (1);
        if(pid == 0)
        {
            if(fd_in != 0)
            {
                dup2(fd_in,0);
                close(fd_in);
            }
            if(cmds[i + 1])
            {
                close(fd[0]);
                dup2(fd[1],1);
                close(fd[1]);
            }
            execvp(cmds[i][0],cmds[i]);
            exit(1);
        }
        else
        {
            if(fd_in != 0)
                close(fd_in);
            if(cmds[i + 1])
            {
                close(fd[1]);
                fd_in = fd[0];
            }
        }
        i++;
    }
    while(wait(NULL) > 0);
    return (0);
}