#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>


#define maxel 100

void freearr(char **arr, int count){
    int i;
    for (i = 0; i < count; i++){
        free(arr[i]);
    }
    free(arr);
}

void addword (int len, char *word, char **elems, int *count, int *buf){
    if (*count >= *buf - 1){
        *buf *= 2;
        elems = realloc (elems, (*buf) * (sizeof(char *)));
        elems[*count] = (char *) malloc ((len + 1) * sizeof(char));
        strncpy (elems[*count], word, len+1);
        (*count)++;
    } else {
        elems[*count] = (char *) malloc ((len + 1) * sizeof(char));
        strncpy (elems[*count], word, len+1);
        (*count)++;
    }
}


int enter_text (char **elems, int *count, int *buf){
    char c = 0;
    int buff = maxel, flag, len = 0, fq = 0;
    char *word = malloc(buff * sizeof(char));
    while (c != '\n') { // проверка перевода строки
        flag = read(0, &c, 1);
        if (c == '\"') {
            fq = !fq;
        }
        if ((flag == 0) && (*count == 0)){ //EOF in start
            free(word);
            return 1;
        }
        if (flag == -1){
            perror("Problem with entering text"); //problem in read
            free(word);
            return 0;
        }
        if (flag && c != '"') { //not eof
            if ((!isspace(c) || fq) && c != '\n') { //not tab or space or \n
                if (len >= buff){
                    buff *= 2;
                    word = realloc (word, buff * sizeof(char));
                }
                word[len] = c;
                len++;
            }
            else if (len) {
                word[len] = '\0';
                addword(len, word, elems, count, buf);
                len = 0;
            }
        }
    }
    free(word);
    if (fq == 1){
        perror("Problem with quotes");
    }
    return 0;
}

void exl(char **str){
    int status;
    int pid = fork();
    if(!pid){
        execvp(str[0], str);
        _exit(1);
    }
    else {
        waitpid(pid, &status, 0);
        if(!WIFEXITED(status)) {
            perror("Child crushed");
        }
    }
}

void cdr(char **cdr, int count){
    if (count == 2){
        chdir(cdr[1]);
    } else if (count == 1) {
        chdir(getenv("HOME"));
    } else {
        printf("count:%d\n", count);
        fprintf(stderr, "Error with args");
    }
}

int main(){
    int count = 0, buf = maxel;
    const char *exits = "exit\0";
    const char *cdir = "cd\0";
    for (;;){
        char **mainels = malloc(buf * sizeof(char *));
        write(1, ">", 1);
        if (enter_text(mainels, &count, &buf) == 1) { //EOF check
            freearr(mainels, count);
            exit(0);
        }
        mainels[count] = NULL;
        if (mainels[0] && strncmp(exits, mainels[0], strlen(exits)) == 0) { //exit check
            freearr(mainels, count);
            exit(0);
        }
        if (mainels[0] && mainels != NULL){
            if (strncmp(cdir, mainels[0], strlen(cdir)) == 0){
                cdr(mainels, count);
            }
            else {
                exl(mainels);
            }
        }
        if (mainels != NULL) freearr(mainels, count);
        count = 0;
    }
}
