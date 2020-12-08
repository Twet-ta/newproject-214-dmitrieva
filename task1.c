#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc,char *argv[]){
    if (argc == 5){
         int fo = 0, fd[2], status;
         fo = open(argv[4], O_CREAT | O_APPEND | O_RDONLY);
         pipe(fd);
         pid = fork();
         if(!pid){ //child
             dup2();
             _exit(0);
         }
         else{
             waitpid(pid, &status, 0);
             if (WIFEXITED(status)){
             }
         }
         
    }
    else{
        fprintf(stderr, "Error");
    }
}
