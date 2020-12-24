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
int m = 0;

void handler(int sig) {
    flag_kill = 1;
    signal(SIGINT, handler);
}

void printmain(char **man, int count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        printf("%s\n", man[i]);
        fflush(stdout);
    }
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
        case ';':
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
    free(elems[(*count)-1]);
    (*count)--;
}

int enter_text (char **mainels, int *count, int *buf, int *flag_and, int *flag_or, int *flag_next, int *flag_save, int fd)
{
    char c = 0, prc = 0, c1;
    int buff = MAXEL, flag, len = 0, fq = 0;
    char *word = malloc(buff * sizeof(char));
    while ((c != '\n') && (*flag_and != 1) && (*flag_or != 1) && (*flag_next != 1)) { // проверка перевода строки
        flag = read(fd, &c, 1);
        if (c == '\"') {
            fq = !fq;
        }
        if ((flag == 0) && ((*count == 0) && (!(*flag_save)))) { //EOF in start
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
                    case ';':
                        *flag_next = 1;
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
                        if (prc == '&') {
                            *flag_and = 1;
                            deleteword(mainels, count);
                        }
                        else{
                            word[0] = '\\';
                            word[1] = '&';
                            word[2] = '\0';
                            len = 2;
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        break;
                    case '|':
                        if (prc == '|') {
                            *flag_or = 1;
                            deleteword(mainels, count);
                            len = 0;
                        }
                        else{
                            word[0] = '\\';
                            word[1] = '|';
                            word[2] = '\0';
                            len = 2;
                            addword(len, word, mainels, count, buf);
                            len = 0;
                        }
                        break;
                    default  :
                        break;
                }
            }
        }
        prc = c;
        if (c == '\n' || c == ';') {
            *flag_save = 0;
        } else {
            *flag_save = 1;
        }
    }
    free(word);
    if (fq == 1) {
        perror("Problem with quotes");
    }
    return 0;
}

void cdr(char **cdr, int count, int *flag_cd){
    if (count == 2){
        chdir(cdr[1]);
    } else if (count == 1) {
        chdir(getenv("HOME"));
    } else {
        printf("count:%d\n", count);
        fprintf(stderr, "Error with args");
        *flag_cd = 1;
    }
}

void check_err_fon(int const count, CMD const *cmd, int const it, int *flag, int flag_and, int flag_or) {
    int i = 0;
    while (i < it){
        if (i + 1 != it) {
            if (cmd[i].f == 1) { //error with &
                *flag = 2;
                break;
            }
        } else {
            if (cmd[i].f == 1) {
                if ((flag_and == 1) || (flag_or == 1)) {
                    *flag = 2; //err
                } else {
                    *flag = 1; //flag fon
                }
            }
        }
        ++i;
    }
}

CMD *get_commands(char **mainels, const int count, int *it, int fdin, int fdout) {
    CMD *commands = calloc(count + 1, sizeof(CMD));
    commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
    commands[*it].fd_in = fdin;
    commands[*it].fd_out = fdout;
    commands[*it].f = 0;
    int argv_counter = 0;
    for (int i = 0; i < count; ++i) {
        if (strcmp("\\|", mainels[i]) == 0) {
            (*it)++;
            argv_counter = 0;
            commands[*it].argv = (char **) calloc(count + 1, sizeof(char *));
            commands[*it].fd_in = fdin;
            commands[*it].fd_out = fdout;
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

void conv(int const count, CMD const *cmd, int const it, int is_fon, int *pid, int *status, int fdin, int fdout) {
    if (it - 1 > count) {
        printf("fb!\n");
        fflush(stdout);
        _exit(0);
    }
    if (flag_kill == 1) flag_kill = 0;
    int i = 0, fd[2];
    *status = 0;
    if (it == 1) {
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
                close(1);
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
                close(fdin);
            }
            while(i < it) {
                pipe(fd);
                if (!fork()) {
                    if (i + 1 != it) { // не последний элемент
                        if (cmd[i].fd_out == fdout) { // стандартный вывод
                            dup2(fd[1], fdout);
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
                    if (is_fon) {
                        close(1);
                    }
                    execvp(cmd[i].argv[0], cmd[i].argv);
                    _exit(1);
                }
                dup2(fd[0], fdin);
                close(fd[1]);
                i++;
            }
            while (wait(NULL) != -1) {} // В ребенке ждем завершения всех внуков
            _exit(1);
        }
    }
    if (!is_fon) { // Это не фоновый процесс
        while (waitpid(*pid, status, 0) > 0) {
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

void save_text(int fd_in, int fd) {
    int flag = 0;
    char c;
    flag = read(fd_in, &c, 1);
    write(fd, &c, 1);
    while ((flag > 0) && (c != '\n')){
        flag = read(fd_in, &c, 1);
        write(fd, &c, 1);
    }
}

void skip_text(int *flag_save, int *flag_next, int fd){
    int flag = 0;
    char c;
    flag = read(fd, &c, 1);
    while ((flag > 0) && (c != '\n') && (c != ';')){
        flag = read(fd, &c, 1);
    }
    if (c == ';') *flag_next = 1;
    if (c == '\n') {
        *flag_save = 0;
        close(fd);
    }
}

int start(int fd_in, int fd_out, int fd_arg, int *flag, int *pid, int *stat) {
    int status, status2, count = 0, buf = MAXEL;
    int flag_cd, flag_and = 0, flag_or = 0, flag_next = 0;
    const char *exits = "exit\0";
    const char *cdir = "cd\0";
    char c;
    int fd[2];
    int flag_enter = 0, flag_save = 0;
    do {
        flag_and = 0; //флаг наличия &&
        flag_or = 0; //флаг наличия ||
        flag_next = 0; //флаг наличия ;
        flag_cd = 0; //флаг наличия проблем с cd
        char **mainels = malloc(buf * sizeof(char *));
        *flag = 0;
        if (!flag_save) { //перед этим был \n или мы только начали)
            flag_enter = enter_text(mainels, &count, &buf, &flag_and, &flag_or, &flag_next, &flag_save, fd_in);
            if (flag_save) {
                pipe(fd);
                save_text(fd_in, fd[1]);
                close(fd[1]);
            }
        } else {
            flag_enter = enter_text(mainels, &count, &buf, &flag_and, &flag_or, &flag_next, &flag_save, fd[0]);
            if (!flag_save) {
                close(fd[0]);
            }
        }
        if (flag_enter) { //EOF check
            freearr(mainels, count);
            signal(SIGTERM, SIG_IGN);
            kill(0, SIGTERM);
            printf("EOF\n");
            if (flag_save) close(fd[0]);
            return 1;
        }
        mainels[count] = NULL;
        //printmain(mainels, count);
        if (mainels[0] && strncmp(exits, mainels[0], strlen(exits)) == 0) { //exit check
            freearr(mainels, count);
            signal(SIGTERM, SIG_IGN);
            kill(0, SIGTERM);
            printf("Goodbye\n");
            if (flag_save) close(fd[0]);
            return 1;
        }
        if (mainels[0] && mainels != NULL) {
            if (strncmp(cdir, mainels[0], strlen(cdir)) == 0) { //cdir check! //change this function!!
                cdr(mainels, count, &flag_cd);
                if (flag_cd) {
                    if (flag_and) {
                        skip_text(&flag_save, &flag_next, fd[0]);
                        flag_and = 0;
                    }
                }
                else {
                    if (flag_or) {
                        skip_text(&flag_save, &flag_next, fd[0]);
                        flag_or = 0;
                    }
                }
            } else {
                int it = 0; // struct array counter
                CMD *commands = get_commands(mainels, count, &it, fd_arg, fd_out);
                check_err_fon(count, commands, it, flag, flag_and, flag_or);
                if (*flag == 2) {
                    if(flag_save) skip_text(&flag_save, &flag_next, fd[0]);
                    flag_and = 0;
                    flag_or = 0;
                }
                else {
                    conv(count, commands, it, *flag, pid, &status2, fd_arg, fd_out);
                    if (!(*flag)){
                        if (WIFEXITED(status2) && (!WEXITSTATUS(status2))) {
                            if (flag_or) {
                                skip_text(&flag_save, &flag_next, fd[0]);
                                flag_or = 0;
                            }
                            *stat = 1;
                        }
                        else {
                            if (flag_and) {
                                skip_text(&flag_save, &flag_next, fd[0]);
                                flag_and = 0;
                            }
                            *stat = 0;
                        }
                    } else if (flag_or || flag_and) {
                        *flag = 0; // на всякий случай еще одна проверка на ошибку с фоном
                    }
                }
                /*if (flag == 1) {
                    printf("[%d] \n", pid);
                    add_pid(pid, arr_pid, &buf_pid, &count_pid);
                }*/
                freecmd(commands, it);
            }
        }
        if ((count == 0) && (flag_and) && (stat)){
            skip_text(&flag_save, &flag_next, fd[0]);
            flag_and = 0;
        }
        if ((count == 0) && (flag_or) && (!stat)){
            skip_text(&flag_save, &flag_next, fd[0]);
            flag_or = 0;
        }
        if (mainels != NULL) freearr(mainels, count);
        count = 0;
    } while (flag_save);
    return 0;
}

int skeep_main(int fd, char *c) {
    char r = ' ';
    int count = 1;
    while ((r != '\n') && (r != ';') && (count || (r != ')'))){
        read(fd, &r, 1);
        if (r == '(') count++;
        if (r == ')') count--;
    }
    if (count) return 1;
    return 0;
}

int brs(int fdin, int *fdi, int *stat_brs) {
    int flag;
    char r, c = '\n';
    int fd_in[2];
    int fd_out[2];
    int fd1, fd0, flag_pipe = 0;
    int new_fd[2];
    int flag_fon, pid;
    int brs_cnt = 0;
    int fdi_next = 0;
    int prev_fdin = fdin;
    int flag_fdin = 0;
    int flag_pipe2 = 0;
    int fd_buf, stat = 1;
    pipe(fd_out);       // буфер вывода
    fd1 = fd_out[1];
    fd0 = fd_out[0];
    while (c != ')') {
        if (!flag_fdin) fdin = prev_fdin;
        pipe(fd_in);//буфер ввода
        do {
            read(0, &c, 1);
            if ((flag_pipe) && (c == '|')) {
                flag_pipe = 0;
                r = '|';
                write(fd_in[1], &r, 1);
            }
            if (c == '(') {
                if (flag_pipe) {
                    write(fd_in[1], "\n", 1);
                } else {
                    write(fd_in[1], "\n", 1); ///change!
                }
                if (!flag_pipe2) {
                    flag = start(fd_in[0], fd1, fdin, &flag_fon, &pid, &stat);
                } else {
                    while (read(fdin, &r, 1) > 0) write(fd1, &r, 1);
                }
                if(flag){
                    return 1;
                }
                pipe(new_fd);
                fd_buf = fd0;
                close(fd1);
                fd0 = new_fd[0];
                fd1 = new_fd[1];
                if ((flag_pipe) || (flag_pipe2)) {
                    //if (flag_pipe2) while (read(fd_buf, &r, 1) > 0) write(1, &r, 1);
                    flag = brs(fd_buf, &fdi_next, &stat); //куда записывает вывод, откуда читает аргументы для команды
                    //if (flag_pipe2) while (read(fd1, &r, 1) > 0) write(1, &r, 1);
                    if (flag) {
                        return 1;
                    }
                    flag_pipe = 0;
                    flag_pipe2 = 0;
                    read(0, &c, 1);
                    while (isspace(c) && (c != '\n')&& (c != ';')) read(0, &c, 1);
                    if (c == '|') {
                        fdin = fdi_next;
                        flag_fdin = 1;
                        read(0, &c, 1);
                        flag_pipe2 = 1;
                    } else {
                        while(read(fdi_next, &r, 1) > 0) write(fd1, &r, 1);
                        close(fdi_next);
                    }
                } else {
                    brs_cnt++;
                    write(fd_in[1], &r, 1);
                }
                close(fd_buf);
            } else {
                if (flag_pipe){
                    write(fd_in[1], "|", 1);
                    flag_pipe = 0;
                }
            }
            if ((c != '|') && (c != ')') && (c != '(')) {
                write(fd_in[1], &c, 1);
            } else {
                if (c == '|') flag_pipe = 1;
            }
            if ((c == ')') && (brs_cnt)) {
                brs_cnt--;
                write(fd_in[1], &c, 1);
                c = ' ';
            }
        } while ((c != '\n') && (c != ';') && (c != ')'));
        if (c == '\n') {
            printf ("Problem with brackets");
            return 1;
        }
        write(fd_in[1], ";", 1);
        close(fd_in[1]);
        flag = start(fd_in[0], fd1, fdin, &flag_fon, &pid, &stat);
        if(flag){
            return 1;
        }
        close(fd_in[0]);
        if (flag_fdin) flag_fdin = 0;
    }
    close(fd1);
    *fdi = fd0;
    *stat_brs = stat;
    return 0;
}


int main() {
    int flag = 0; //eof
    char c = '\n';
    int fd_in[2];//буфер ввода
    int new_fd[2];//буфер вывода новый на случай встречи скобочек
    int fd_out[2];//буфер вывода
    int flag_pipe = 1;
    int fd1, fd0;//дескрипторы записи и чтения для буфера вывода
    int buf_pid = MAXEL, count_pid = 0, pid = 0;
    int *arr_pid = calloc(buf_pid,  sizeof(int));
    int flag_fon = 0;
    int status;
    char r, pr1 = ' ', pr2 = ' ';
    int brs_cnt = 0, fdi_next, fdin = 0, flag_fdin = 0, flag_pipe2 = 0, flag_and = 0, flag_or = 0;
    int fd_buf, stat = 1, flag_pipe3, flag_pipe4;
    signal(SIGINT, handler);
    write(1, "Hello!\n", 7);
    for(;;) {
        if (!flag_fdin) fdin = 0;
        if (c == '\n') {
            write(1, ">", 1);
        }
        //буфер ввода как в старте сделать!!!
        if (count_pid > 0) {
            while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                if (WIFEXITED(status)) {
                    printf("[%d] completed, status = %d\n", pid, WEXITSTATUS(status));
                }
                else {
                    printf ("[%d] killed\n", pid);
                }
                for (int i = 0; i < count_pid; ++i) {
                    if (arr_pid[i] == pid) {
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
        flag_pipe = 2;
        flag_pipe3 = 0;
        flag_pipe4 = 0;
        pipe(fd_in);
        pipe(fd_out);
        fd1 = fd_out[1];
        fd0 = fd_out[0];
        do {
            read(0, &c, 1);
            if ((flag_pipe == 3) && (c == '|')) {
                flag_pipe = 0;
                r = '|';
                write(fd_in[1], &r, 1);
                pr2 = pr1;
                pr1 = r;
            } else if ((flag_pipe == 3) && (c != '|')) flag_pipe = 1;
            if ((flag_pipe2) && (c == '|')) {
                flag_pipe2 = 0;
                flag_pipe = 0;
                //while (read(fdin, &r, 1) > 0) write(fd1, &r, 1);
                flag_pipe3 = 0;
                fdin = 0;
                flag_fdin = 0;
                r = '|';
                write(fd_in[1], &r, 1);
                pr2 = pr1;
                pr1 = r;
            } else if (flag_pipe2){
                flag_pipe2 = 0;
                flag_pipe3 = 1;
                if (c == '\n')  flag_pipe4 = 1;
            }
            if (c == '(') {
                write(fd_in[1], "\n", 1);
                if (!flag_pipe3) {
                    flag = start(fd_in[0], fd1, fdin, &flag_fon, &pid, &stat);
                } else{
                    while (read(fdin, &r, 1) > 0) write(fd1, &r, 1);
                }
                if (((pr1 == '&') && (pr2 == '&') && stat) || ((pr1 == '|') && (pr2 == '|') && !stat)){
                    if(skeep_main(0, &c) == 1){
                        printf("Problem with brackets");
                        exit(0);
                    }
                } else {
                    if(flag) {
                        free(arr_pid);
                        exit(0);
                    }
                    pipe(new_fd);
                    fd_buf = fd0;
                    close(fd1);
                    fd0 = new_fd[0];
                    fd1 = new_fd[1];
                    //if (flag_pipe2) while (read(fd_buf, &r, 1) > 0) write(1, &r, 1);
                    flag = brs(fd_buf, &fdi_next, &stat); //откуда читает аргументы для команды, куда записывает вывод
                    if(flag) {
                        free(arr_pid);
                        exit(0);
                    }
                    flag_pipe = 0;
                    flag_pipe2 = 0;
                    read(0, &c, 1);
                    while (isspace(c) && (c != '\n')&& (c != ';')) read(0, &c, 1);
                    if (c == '|') {
                        fdin = fdi_next;
                        flag_fdin = 1;
                        flag_pipe2 = 1;
                    } else {
                        while(read(fdi_next, &r, 1) > 0) write(fd1, &r, 1);
                        close(fdi_next);
                    }
                    close(fd_buf);
                }
            }
            if ((c == ')') && (brs_cnt)) {
                brs_cnt--;
            } else {
                if (c == ')') {
                    printf ("Problem with brackets");
                    exit (0);
                }
            }
            if (!isspace(c) && (c != '(') && (c != ')')){
                pr2 = pr1;
                pr1 = c;
            }
            if ((c != '|') && (c != '(') && (c != ')')) {
                if (flag_pipe2) flag_pipe2 = 0;
                if (flag_pipe == 1) {
                    if (!isspace(c)){
                        flag_pipe = 0;
                        r = '|';
                        write(fd_in[1], &r, 1);
                        pr2 = pr1;
                        pr1 = '|';
                    }
                }
                write(fd_in[1], &c, 1);
            } else {
                if ((c == '|') && (!flag_pipe2)) {
                    flag_pipe = 3;
                }
            }
            if ((c == ')') && (brs_cnt)) {
                write(fd_in[1], &c, 1);
                brs_cnt--;
                c = ' ';
            }
        } while ((c != '\n') && (c != ';'));
        close(fd_in[1]);
        //while(read(fdin, &r, 1) > 0) write(1, &r, 1);
        if (!flag_pipe4) flag = start(fd_in[0], fd1, fdin,  &flag_fon, &pid, &stat);// read, out, arg
        else while (read(fdin, &r, 1) > 0) write(fd1, &r, 1);
        if(flag){
            free(arr_pid);
            exit(0);
        }
        if (flag_fon == 1) {
            printf("[%d] \n", pid);
            add_pid(pid, arr_pid, &buf_pid, &count_pid);
        }
        close(fd_in[0]);
        close(fd1);
        while(read(fd0, &r, 1) > 0) write(1, &r, 1);
        close(fd0);
        if (flag_fdin) {
            flag_fdin = 0;
            close(fdin);
        }
        //close(fd_out[0]);/// новое условие для выхода из функции старт
    }
}
