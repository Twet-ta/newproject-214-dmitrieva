#include<stdio.h>
#include<stdlib.h>

int main(){
    FILE *fr1 = fopen("tr1.txt","r");
    FILE *fr2 = fopen("tr2.txt","r");
    char *str = malloc(1);
    int flag = 0, size = 0, numb = 0, i = 0, p = 0, c;
    while ((c = getc(fr1)) != EOF){
        p = getc(fr2);
        if (c != p) flag = 1;
        if ((c != '\n')&&((p != EOF))){
            if (size == numb){
                size = size*2+1;
                str = realloc(str, size);
                if (str == NULL) {
                    fprintf(stderr, "Error with size");
                    return 1;
                }
            }
            str[numb] = c;
            numb++;
        }
        else{
            if (flag) break;
            numb = 0;
        }
    }
    printf("%d",flag);
    if ((flag)||(getc(fr2) != EOF)) {
        for (i = 0; i < numb; i++) {
            putchar(str[i]);
        }
        while ((c != EOF) && (c != '\n')) {
            putchar(c);
            c = getc(fr1);
        }
    }
    free(str);
    return 0;
}
