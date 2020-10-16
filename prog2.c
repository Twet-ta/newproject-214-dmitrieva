#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int sum = 0;

struct tree{
    char *arr;
    int num;
    struct tree *left;
    struct tree *right;
};

struct list{
    char *arr;
    int num;
    float ch;
    struct list *nextlist;
};

void addtree (struct tree **head, char *word, int len){
    if ((*head) == NULL){
        (*head) = malloc(sizeof(struct tree));
        (*head)->arr = malloc(len);
        strncpy((*head)->arr, word, len);
        (*head)->num = 1;
        (*head)->left = (*head)->right = NULL;
    }
    else{
        if (strcmp((*head)->arr, word) < 0){
            addtree(&((*head)->left), word, len);
        }
        else{
            if (strcmp((*head)->arr, word) == 0) {
                (*head)->num++;
            }
            else{
                addtree(&((*head)->right), word, len);
            }
        }
    }
}

void getwords (FILE *f, struct tree **h){
    int count = 0, size = 25, c;
    char *words = NULL; 
    char *symb = NULL; 
	while ((c = fgetc(f)) != EOF){
        if (!count) {
            size = 25;
            words = malloc(size);
        }
        if(size <= count){
            size *= 2;
            words = realloc(words,size);
        }
        if (c == ' ' || c == '\n') {
            if (count) {
                words[count] = '\0';
                addtree(h, words, count+1);
                sum++;
            }
            free(words);
            words = NULL;
            count = 0;
        }
        else{
            if (isalpha(c)){
                words[count] = (char) c;
                count++;
            }
            else {
                if (count != 0) {
                    words[count] = '\0';
                    addtree(h, words, count+1);
                    sum++;
                }
                if (ispunct(c)){
                    symb = malloc(2);
                    symb[0] = c;
                    symb[1] = '\0';
                    addtree(h, symb, 2);
                    free(symb);
                    symb = NULL;
                    sum++;
                }
                free(words);
                words = NULL;
                count = 0;
            }
        }
    }
    if (count) {
        words[count] = '\0';
        addtree(h, words, count+1);
        free(words);
        words = NULL;
        sum++;
    }
}

///ищет нужное место для слова из head
void addlist(struct tree **head, struct list **point){
    struct list **nod;
    struct list *nod1;
    nod = point;
    float chmain = ((*head)->num);
    chmain /= sum;
    if ((*point) == NULL) {
        *point = malloc(sizeof(struct list));
        (*point)->arr = malloc(strlen((*head)->arr)+1);
        strcpy((*point)->arr, (*head)->arr);
        (*point)->ch = chmain;
        (*point)->num = (*head)->num;
        (*point)->nextlist = NULL;
    }
    else{
        if ((*point)->ch > chmain) addlist(head, &((*nod)->nextlist));
        else{
            nod1 = malloc(sizeof(struct list));
            nod1->nextlist = *point;
            *point = nod1;
            nod1->arr = malloc(strlen((*head)->arr)+1);
            strcpy(nod1->arr, (*head)->arr);
            nod1->ch = chmain;
            nod1->num = (*head)->num;
        }
    }
}

///перебирает слова из дерева
void createlist(struct tree *head, struct list **point){
    if (head){
        createlist((head->left), point);
        addlist(&head, point);
        createlist((head->right), point);
    }
}

void printlist(struct list *l, FILE *f){
    if (l){
        fprintf(f,"%s %d %f\n", l->arr, l->num, l->ch);
        printlist((l->nextlist), f);
    }
}

void freetree(struct tree *head){
    if (head != NULL){
        freetree(head->left);
        freetree(head->right);
        free(head->arr);
        head->arr = NULL;
        free(head);
        head = NULL;
    }
}

void freelist(struct list *l){
    if (l) {
        freelist(l->nextlist);
        free(l->arr);
        l->arr = NULL;
        free(l);
        l = NULL;
    }
}

int main(int argc, char *argv[]) {
    FILE *fin;
    fin = stdin;
    FILE *fout;
    fout = stdout;
    for (int i = 1; i < argc; ++i){
        if (strcmp(argv[i], "-o") == 0) fout = fopen(argv[i+1],"w");
        if (strcmp(argv[i], "-i") == 0) fin = fopen(argv[i+1],"r");
    }
    struct tree *h = NULL;
    getwords(fin, &h);
    struct list *finlist = NULL;
    createlist(h, &finlist);
    printlist(finlist, fout);
    freetree(h);
    freelist(finlist);
    fclose(fin);
    fclose(fout);
    return 0;
}

