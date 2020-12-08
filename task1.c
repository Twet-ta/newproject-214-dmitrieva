#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

int conv (const char *str, int fd_in, int fd_out)
{
    int pid = fork();
    if (fd_in != 0) {
        dup2(fd_in, 0);
        close(fd_in);
    }
    if (fd_out != 1) {
        dup2(fd_out, 1);
        close(fd_out);
    }
    if (pid == 0) {
        execlp(str, str, NULL);
        _exit(1);
    } 
    else if (pid > 0) {
        int status;
        wait(&status);
        if (!WIFEXITED(status)) {
            return 1;
        } 
        else if (WEXITSTATUS(status) != 0) {
            return 1;
        } 
        else {
            return 0;
        }
    }
    return 1;
}

int main(int argc,char *argv[]){
    if (argc != 5) return 0;
    int fo = open(argv[4], O_RDONLY);
    int fd[2], status, pid;
    int old_stdin, old_stdout;
    old_stdout = dup(1);
    old_stdin = dup(0);
    pid = fork();
    pipe(fd);
    if(!pid){ //child
        if (!conv(argv[1], fo, fd[1])) {
            conv(argv[2], fo, fd[1]);
        }
    } 
    else 
    {
        wait(NULL);
    }
    pid = fork();
    if (!pid) { // pr3
        conv(argv[3], fd[0], old_stdout);
    }
    close(old_stdin);
    close(old_stdout);
    return 0;
}



