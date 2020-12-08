#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAXEL 100

typedef struct cmd {
    char **argv;
    int fd_in;
    int fd_out;
    int f;
} CMD;

int flag_kill = 0;

void handler(int sig) {
    flag_kill = 1;
    signal(SIGINT, handler);
}

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

void printarr(int *arr, int count)
{
    for (int i = 0; i < count; i++) {
        printf("%d\n",arr[i]);
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
    printf("f:%d\n", cmd.f);
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
        case '&':
            return 1;
        default:
            return 0;
    }
}

void add_pid(int pid, int *arr_pid, int *buf_pid, int *count_pid){
    if (*count_pid == *buf_pid){
        *buf_pid *= 2;
        arr_pid = realloc (arr_pid, (*buf_pid) * (sizeof(int)));
        arr_pid[*count_pid] = pid;
        (*count_pid)++;
    }
    else {
        arr_pid[*count_pid] = pid;
        (*count_pid)++;
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
        if ((flag == 0) && (*count == 0)) { //EOF in start
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
                            word[0] = '\\';
                            word[1] = '>';
                            word[2] = '>';
                            word[3] = '\0';
                            len = 3;
                            deleteword(mainels, count);
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        else{
                            word[0] = '\\';
                            word[1] = '>';
                            word[2] = '\0';
                            len = 2;
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        break;
                    case '<':
                        word[0] = '\\';
                        word[1] = '<';
                        word[2] = '\0';
                        len = 2;
                        addword(len, word, mainels, count, buf);
                        len = 0;
                        break;
                    case '&':
                        word[0] = '\\';
                        word[1] = '&';
                        word[2] = '\0';
                        len = 2;
                        addword(len, word, mainels, count, buf);
                        len = 0;
                        break;
                    case '|':
                        word[0] = '\\';
                        word[1] = '|';
                        word[2] = '\0';
                        len = 2;
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

void check_err_fon(int const count, CMD const *cmd, int const it, int *flag) {
    int i = 0;
    while (i < it){
        if (i + 1 != it) {
            if (cmd[i].f == 1) { //error with &
                *flag = 2;
                break;
            }
        } else {
            if (cmd[i].f == 1) *flag = 1; //flag fon
        }
        ++i;
    }
}

CMD *get_commands(char **mainels, const int count, int *it) {
    CMD *commands = calloc(count + 1, sizeof(CMD));
    commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
    commands[*it].fd_in = 0;
    commands[*it].fd_out = 1;
    commands[*it].f = 0;
    int argv_counter = 0;
    for (int i = 0; i < count; ++i) {
        if (strcmp("\\|", mainels[i]) == 0) {
            (*it)++;
            argv_counter = 0;
            commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
            commands[*it].fd_in = 0;
            commands[*it].fd_out = 1;
            commands[*it].f = 0;
        } else if (strcmp("\\>", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0660);
            commands[*it].fd_out = fd;
            i++;
        } else if (strcmp("\\>>", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_APPEND | O_WRONLY, 0660);
            commands[*it].fd_out = fd;
            i++;
        } else if (strcmp("\\<", mainels[i]) == 0) {
            int fd = open(mainels[i + 1], O_CREAT | O_APPEND | O_RDONLY);
            commands[*it].fd_in = fd;
            i++;
        } else if (strcmp("\\&", mainels[i]) == 0) {
            commands[*it].f = 1;
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

void conv(int const count, CMD const *cmd, int const it, int is_fon, int *pid) {
    if (it - 1 > count) {
        printf("fb!\n");
        fflush(stdout);
        _exit(0);
    }
    int i = 0, fd[2];
    if (i + 1 == it) {
        *pid = fork();
        if (!(*pid)) {
                if (cmd[i].fd_out != 1) { // для любого элемента можем перенаправить вывод
                    dup2(cmd[i].fd_out, 1);
                    close(cmd[i].fd_out);
                }
                if (cmd[i].fd_in != 0) { // для любого элемента можем перенаправить ввод
                    dup2(cmd[i].fd_in, 0);
                    close(cmd[i].fd_in);
                }
                //close(fd[0]); close(fd[1]);
                if (is_fon) {
                    signal(SIGINT, SIG_IGN);
                }
                execvp(cmd[i].argv[0], cmd[i].argv);
                _exit(1);
            }
    }
    else {
        *pid = fork();
        if (!(*pid)) { //father
            if (is_fon) {
                signal(SIGINT, SIG_IGN);
            }
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
                    _exit(0);
                }
                dup2(fd[0], 0);
                close(fd[1]);
                i++;
            }
            while (wait(NULL) != -1) {} // В ребенке ждем завершения всех внуков
            _exit(0);
        }
    }
    if (!is_fon) { // Это не фоновый процесс
        while (waitpid(*pid, NULL, 0) > 0) {
            usleep(1000);
            if (flag_kill == 1) {
                flag_kill = 0;
                printf("\nkill [%d]\n", *pid);
                fflush(stdout);
                kill(*pid, SIGKILL);
            }
        }
    }
}

int main() {
    write(1, "Hello!\n", 7);
    int pid, status, pid_i = 1, count = 0, buf = MAXEL, flag = 0, buf_pid = MAXEL, count_pid = 0;
    int *arr_pid = calloc(buf_pid,  sizeof(int));
    const char *exits = "exit\0";
    const char *cdir = "cd\0";
    signal(SIGINT, handler);
    for (;;) {
        flag_kill = 0;
        // Первым делом проверяем завершившиеся процессы
        if (count_pid > 0) {
            while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                if (WIFEXITED(status)) {
                    printf("[%d] completed, status = %d\n", pid, WEXITSTATUS(status));
                }
                for (int i = 0; i < count_pid; ++i) {
                    if (arr_pid[i] == pid_i) {
                        arr_pid[i] = 0;
                    }
                }
                int p, q;
                for (p = 0, q = 0; p < count_pid;) {
                    if (!arr_pid[p]) { // == 0
                        p++;
                    } else { // != 0
                        arr_pid[q] = arr_pid[p];
                        p++;
                        q++;
                    }
                }
                count_pid = q;
            }
        }
        char **mainels = malloc(buf * sizeof(char *));
        flag = 0;
        write(1, ">", 1);
        if (enter_text(mainels, &count, &buf) == 1) { //EOF check
            freearr(mainels, count);
            signal(SIGTERM, SIG_IGN);
            kill(0, SIGTERM);
            free(arr_pid);
            printf("EOF\n");
            exit(0);
        }
        mainels[count] = NULL;
        if (mainels[0] && strncmp(exits, mainels[0], strlen(exits)) == 0) { //exit check
            freearr(mainels, count);
            signal(SIGTERM, SIG_IGN);
            kill(0, SIGTERM);
            free(arr_pid);
            printf("Goodbye\n");
            exit(0);
        }
        if (mainels[0] && mainels != NULL) {
            if (strncmp(cdir, mainels[0], strlen(cdir)) == 0) { //cdir check!
                cdr(mainels, count);
            } else {
                int it = 0; // struct array counter
                CMD *commands = get_commands(mainels, count, &it);
                check_err_fon(count, commands, it, &flag);
                if (flag == 2) {
                    fprintf(stderr, "Error with &");
                }
                else {
                    conv(count, commands, it, flag, &pid);
                }
                if (flag) {
                    printf("[%d] \n", pid);
                    add_pid(pid, arr_pid, &buf_pid, &count_pid);
                }
                freecmd(commands, it);
            }
        }
        if (mainels != NULL) freearr(mainels, count);
        count = 0;
    }
}
