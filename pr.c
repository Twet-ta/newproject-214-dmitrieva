#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXEL 100

typedef struct cmd {
    char **argv;
    int fd_in;
    int fd_out;
} CMD;

void freearr(char **arr, int count)
{
    for (int i = 0; i < count; i++) {
        free(arr[i]);
    }
    free(arr);
}

void freecmd(CMD *commands, int it)
{
    for (int i = 0; i < it; ++i) {
        free(commands[i].argv);
        commands[i].argv = NULL;
    }
    free(commands);
}

void printarr(char **arr, int count)
{
    for (int i = 0; i < count; i++) {
        printf("%s\n",arr[i]);
    }
}

void printcmd(CMD cmd) {
    int i = 0;
    while(cmd.argv[i] != NULL) {
        printf("%s ", cmd.argv[i]);
        i++;
    }
    printf("out:%d ", cmd.fd_out);
    printf("in:%d\n", cmd.fd_in);
    fflush(stdout);
}

int is_special(const char c)
{
    switch(c) {
        case '>':
            return 1;
        case '<':
            return 1;
        case '|':
            return 1;
        default:
            return 0;
    }
}

void addword (int len, char *word, char **elems, int *count, int *buf)
{
    if (*count >= *buf - 1) {
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

void deleteword (char **elems, int *count){
    free(elems[*count]);
    (*count)--;
}

int enter_text (char **mainels, int *count, int *buf)
{
    char c = 0, prc = 0;
    int buff = MAXEL, flag, len = 0, fq = 0;
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
        if (flag && c != '\"') { //not eof
            if ( ((!isspace(c) && !is_special(c)) || fq) && c != '\n' ) { //not tab or space or \n or special !!!short logic
                if (len >= buff){
                    buff *= 2;
                    word = realloc (word, buff * sizeof(char));
                }
                word[len] = c;
                len++;
            }
            else if (len) {
                word[len] = '\0';
                addword(len, word, mainels, count, buf);
                len = 0;
            }
            if (is_special(c) && !fq) {
                switch(c) {
                    case '>':
                        if (prc == '>') {
                            word[0] = '>';
                            word[1] = '>';
                            word[2] = '\0';
                            len = 2;
                            deleteword(mainels, count);
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        else{
                            *word = '>';
                            word[1] = '\0';
                            len = 1;
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        break;
                    case '<':
                        *word = '<';
                        word[1] = '\0';
                        len = 1;
                        addword(len, word, mainels, count, buf);
                        len = 0;
                        break;
                    case '|':
                        *word = '|';
                        word[1] = '\0';
                        len = 1;
                        addword(len, word, mainels, count, buf);
                        len = 0;
                        break;
                    default  :
                        break;
                }
            }
        }
        prc = c;
    }
    free(word);
    if (fq == 1) {
        perror("Problem with quotes");
    }
    return 0;
}

void exl(char **str){
    int status, i, f = 1, fd1 = 0, fd2 = 0;
    ///place for change enter
    for (i = 0; str[i] != NULL; i++){
        if (str[i][0] == '>'){
            if (str[i][1] == '>'){
                free(str[i]);
                str[i] = NULL;
                fd1 = open(str[i+1], O_WRONLY | O_APPEND, 0666);
                free(str[i+1]);
                str[i+1] = NULL;
            }
            else{
                free(str[i]);
                str[i] = NULL;
                fd1 = open(str[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0666);
                free(str[i+1]);
                str[i+1] = NULL;
            }
        }
        if (str[i][0] == '<'){
            free(str[i]);
            str[i] = NULL;
            fd2 = open(str[i+1], O_RDONLY, 0666);
            free(str[i+1]);
            str[i+1] = NULL;
        }
        if (fd1 || fd2){
            if(str[i]) free(str[i]);
        }
    }
    if (fd1 == -1 || fd2 == -1){
        perror("Error with files");
        exit(1);
    }
    ///end of place
    if (fd1){
        dup2(fd1, 0);
        close(fd1);
    }
    if (fd2){
        dup2(fd2, 1);
        close(fd2);
    }
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

CMD *get_commands(char **mainels, const int count, int *it) {
    CMD *commands = calloc(count + 1, sizeof(CMD));
    commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
    commands[*it].fd_in = 0;
    commands[*it].fd_out = 1;
    int argv_counter = 0;
    for (int i = 0; i < count; ++i) {
        if (strcmp("|", mainels[i]) == 0) {
            (*it)++;
            argv_counter = 0;
            commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
            commands[*it].fd_in = 0;
            commands[*it].fd_out = 1;
        } else if (strcmp(">", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0660);
            commands[*it].fd_out = fd;
            i++;
        } else if (strcmp(">>", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_APPEND | O_WRONLY, 0660);
            commands[*it].fd_out = fd;
            i++;
        } else if (strcmp("<", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_APPEND | O_RDONLY);
            commands[*it].fd_in = fd;
            i++;
        } else {
            commands[*it].argv[argv_counter] = mainels[i];
            commands[*it].argv[argv_counter + 1] = NULL;
            argv_counter++;
        }
    }
    (*it)++;
    return commands;
}

void conv(int const count, CMD const *cmd, int const it) {
    if (it - 1 > count) {
        printf("fb!\n");
        fflush(stdout);
        _exit(0);
    }
    int old_in, old_out;
    old_out = dup(1);
    old_in = dup(0);
    int i = 0, fd[2];
    while(i < it) {
        pipe(fd);
        if (!fork()) {
            if (i + 1 != it) { // не последний элемент
                if (cmd[i].fd_out == 1) { // стандартный вывод
                    dup2(fd[1], 1);
                }
            }
            if (cmd[i].fd_out != 1) { // для любого элемента можем перенаправить вывод
                dup2(cmd[i].fd_out, 1);
                close(cmd[i].fd_out);
            }
            if (cmd[i].fd_in != 0) { // для любого элемента можем перенаправить ввод
                dup2(cmd[i].fd_in, 0);
                close(cmd[i].fd_in);
            }
            close(fd[0]); close(fd[1]);
            execvp(cmd[i].argv[0], cmd[i].argv);
            _exit(1);
            _exit(1);
        }
        dup2(fd[0], 0);
        close(fd[1]);
        i++;
    }
    while (wait(NULL) != -1) {}
    dup2(old_in, 0);
    dup2(old_out, 1);
    close(old_in);
    close(old_out);
}

int main() {
    write(1, "Hello!\n", 7);
    int count = 0, buf = MAXEL;
    const char *exits = "exit\0";
    const char *cdir = "cd\0";
    for (;;) {
        char **mainels = malloc(buf * sizeof(char *));
        write(1, ">", 1);
        if (enter_text(mainels, &count, &buf) == 1) { //EOF check
            freearr(mainels, count);
            printf("EOF\n");
            exit(0);
        }
        mainels[count] = NULL;
        if (mainels[0] && strncmp(exits, mainels[0], strlen(exits)) == 0) { //exit check
            freearr(mainels, count);
            printf("Goodbye\n");
            exit(0);
        }
        if (mainels[0] && mainels != NULL) { //cdir check!
            if (strncmp(cdir, mainels[0], strlen(cdir)) == 0){
                cdr(mainels, count);
            } else {
                int it = 0; // struct array counter
                CMD *commands = get_commands(mainels, count, &it);
                conv(count, commands, it);
                freecmd(commands, it);
            }
        }
        if (mainels != NULL) freearr(mainels, count);
        count = 0;
    }
}
