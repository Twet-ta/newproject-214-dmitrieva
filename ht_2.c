#include <stdio.h>
#include <stdlib.h>
 
char *mas1;
char *mas2;
 
int main() {
    int size = 0, numb = 0, ch, max = 0;
    while ((ch = getchar()) != EOF) {
        if (ch != '\n') {
            if (numb == size) {
                size = 2 * size + 1;
                mas1 = realloc(mas1, size);
                if (mas1 == NULL) {
                    fprintf(stderr, "Error with size");
                    return 1;
                }
            }
            mas1[numb] = ch;
            numb++;
        } else {
            if (max < numb) {
                max = numb;
                free(mas2);
                mas2 = malloc(size);
                for (numb = 0; numb < max; numb++) {
                    mas2[numb] = mas1[numb];
                }
            }
            free(mas1);
            mas1 = malloc(size);
            numb = 0;
        }
    }
    for (numb = 0; numb < max; numb++) {
        putchar(mas2[numb]);
    }
    putchar('\n');
    free(mas2);
    return 0;
}
