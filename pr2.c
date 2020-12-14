/*2.Написать программу, в которой порождается дочерний процесс. Процесс-отец открывает файл
 * (его имя задано в командной строке), в котором содержатся имена других файлов — по одному в строке,
 * и через неименованный канал передает эти имена дочернему процессу. Дочерний процесс открывает
 * полученный файл, печатает его имя, определяет длину файла, закрывает и результат передает отцу
 * через ТОТ ЖЕ канал. Отец печатает длину соответствующего файла и перевод строки. Когда исходный
 * файл заканчивается, отец закрывает канал и дожидается завершения дочернего процесса, а дочерний
 * процесс, получив сигнал, печатает количество обработанных файлов и завершается. Синхронизацию процессов
 * (при выводе в стандартный канал и работе с неименованным каналом) осуществить с помощью сигналов. */

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int flag_dad = 0; //флаг считывания символа от ребенка
int flag_chld = 0; //флаг отправления символа от родителя
int flag_chld2 = 0; //флаг окончания ввода названия файла от родителя
int flag_chld3 = 0; //флаг завершения дочернего процесса

void sig_dad(int sig) {
    flag_dad = 1;
    signal(SIGUSR2, sig_dad);
}

void sig_chld(int sig) {
    flag_chld = 1;
    signal(SIGUSR1, sig_chld);
}

void sig_chld2(int sig) {
    flag_chld2 = 1;
    signal(SIGINT, sig_chld2);
}

void sig_chld3(int sig) {
    flag_chld3 = 1;
    signal(SIGCHLD, sig_chld3);
}

int main(int argc, char *argv[]){
    if (argc != 2) return 0;
    char c;
    int file = open(argv[1], O_RDONLY);
    int fd[2];
    pipe(fd);
    int pid = fork();
    if (!pid){ //child
        signal(SIGUSR1, sig_chld);
        signal(SIGINT, sig_chld2);
        signal(SIGUSR2, SIG_IGN);
        signal(SIGCHLD, sig_chld3);
        kill(getppid(), SIGUSR2);
        int buff = 100;
        int count = 0;
        int sum = 0;
        int sum_file = 0;
        char t;
        char *str = malloc(buff);
        while (!flag_chld3){
            while (!(flag_chld || flag_chld2)) {
                sleep(1);
                if (flag_chld3) break;
            }
            if (flag_chld3) break;
            if (flag_chld) {
                flag_chld = 0;
                read(fd[0], &c, sizeof(c));
                if (count >= buff) {
                    buff *= 2;
                    str = realloc(str, buff);
                    str[count] = c;
                } else {
                    str[count] = c;
                }
                count++;
            } else {
                flag_chld2 = 0;
                str[count] = 0;
                printf("%s", str);
                fflush(stdout);
                int fl = open(str, O_RDONLY);
                while (read(fl, &t, sizeof(char)) > 0) sum++;
                write(fd[1], &sum, sizeof(sum));
                sum_file++;
                sum = 0;
                close(fl);
                count = 0;
            }
            kill(getppid(), SIGUSR2);
        }
        printf("Sum of file: %d\n", sum_file);
        fflush(stdout);
        free(str);
        _exit(0);
    } else {
        signal(SIGUSR2, sig_dad);
        int sum_dad;
        while (!flag_dad) {
            sleep(1);
        }
        flag_dad = 0;
        while (read(file, &c, sizeof(c)) > 0){
            if (c == '\n'){
                kill(pid, SIGINT);
                while (!flag_dad) {
                    sleep(1);
                }
                flag_dad = 0;
                read (fd[0], &sum_dad, sizeof(sum_dad));
                printf(" Length: %d\n", sum_dad);
                fflush(stdout);
                flag_dad = 0;
            } else {
                write(fd[1], &c, sizeof(char));
                kill(pid, SIGUSR1);
                while (!flag_dad) {
                    sleep(1);
                }
                flag_dad = 0;
            }
        }
        kill(pid, SIGCHLD);
        close(fd[0]);
        close(fd[1]);
        close(file);
        wait(NULL);
    }
    return 0;

}
