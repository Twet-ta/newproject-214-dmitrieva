#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

/* 1.Программа: В командной строке задано имя файла. Оставить в файле только те строки заданного файла,
 * которые содержат хотя бы два одинаковых символа. Никакую дополнительную память для хранения строк и
 * дополнительные файлы использовать нельзя. Для работы с файлом разрешено пользоваться только системными
 * вызовами низкоуровневого ввода-вывода. Для «обрезки» файла до нужной длины использовать системные вызовы
 * truncate() или ftruncate(). */

int c_in_str(char c, int fd, int pos, int str_pos){
    char c2;
    int flag = 0;
    int count = 0;
    lseek(fd, str_pos, SEEK_SET);
    while (read(fd, &c2, sizeof(c2)) > 0){
        if (c2 == '\n'){
            break;  
        } 
        else{
            if (c2 == c) count++;
        }
    }
    if (count >= 2) flag = 1;
    lseek(fd, pos, SEEK_SET);
    return flag;
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 0;
    int fd1 = open(argv[1], O_RDONLY);
    int fd2 = open(argv[1], O_WRONLY);
    int cur_pos = 0; //итератор для указателя в файле на запись - количество записанных символов
    int rd_cur_pos = 1; //итератор для указателя в файле, открытом на чтение
    int str_pos = 0; //указатель на начало строки в файле на запись
    int rd_str_pos = 0; //указатель на начало строки в файле, открытом на чтение
    int flag = 0;
    char c;
    int debug=0;
    while (read(fd1, &c, sizeof(c)) > 0){
        if (c == '\n'){
            if (!flag){
                lseek(fd2, str_pos, SEEK_SET);
                cur_pos = str_pos;
                rd_str_pos = rd_cur_pos;
            } else {
                flag = 0;
                str_pos = cur_pos;
                rd_str_pos = rd_cur_pos;
            }
        } else {
            if (c_in_str(c, fd1, rd_cur_pos, rd_str_pos)) flag = 1;
        }
        write(fd2, &c, sizeof(c));
        rd_cur_pos++;
        cur_pos++;
    }
    ftruncate(fd2, cur_pos);
    return 0;
}

