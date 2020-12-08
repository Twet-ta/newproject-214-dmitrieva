#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>


int
main(int argc, char *argv[])
{
    int fd1 = open(argv[1], O_RDWR);
    int fd2 = open(argv[1], O_RDWR);

    int s, count = 0, curr_count = 0;
    int dig_count = 0, str_count = 0;
    int lose = 0;
    while (read(fd1, &s, 1) > 0) {
        if (s == '\n') {
            if (dig_count == 2) {
                lseek(fd2, curr_count-str_count - 1, SEEK_SET);
                //printf("%d %d\n", count, str_count);
                lose += str_count;
                curr_count -= (str_count + 1);
            }
            str_count = 0;
            dig_count = 0;
        } else if (s <= '9' && s >= '0'){
            dig_count++;
            str_count++;
        } else {
            str_count++;
        }
        write(fd2, &s, 1);
        curr_count++;
        count++;
    }
    count++;
    s = '\0';
    write(fd2, &s, 1);
    ftruncate(fd2, curr_count);
    close(fd1);
    close(fd2);
    return 0;
}
