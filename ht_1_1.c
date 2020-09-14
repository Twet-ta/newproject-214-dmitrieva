#include <stdio.h>
 
int main() {
    int array[1024];
    int x, size;
    x = scanf("%d", &x);
    for (int i = 0; x != EOF; ++i) {
        array[i] = x;
        size = i + 1;
        scanf("%d", &x);
    }
    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            if (array[i] > array[j]) {
                int tmp = array[j];
                array[j] = array[i];
                array[i] = tmp;
            }
        }
    }
    for (int i = 0; i < size; ++i) {
        printf("%d ", array[i]);
    }
    printf("\n");
    return 0;
}
