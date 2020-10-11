#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
 
#define MAXLEN 50
int sum = 0;
 
struct tree{
    char arr[MAXLEN];
    int num;
    struct tree *left;
    struct tree *right;
};
 
struct list{
    char arr[MAXLEN];
    int num;
    float ch;
    struct list *nextlist;
};
 
void addtree (struct tree **head, char *word){
    if ((*head) == NULL){
        (*head) = malloc(sizeof(struct tree));
        strcpy((*head)->arr, word);
        (*head)->num = 1;
        (*head)->left = (*head)->right = NULL;
    }
    else{
        if (strcmp((*head)->arr, word) < 0){
            addtree(&((*head)->left), word);
        }
        else{
            if (strcmp((*head)->arr, word) == 0) {
                (*head)->num++;
            }
            else{
                addtree(&((*head)->right), word);
            }
        }
    }
}
 
void getwords (FILE *f, struct tree **h){
    int count = 0, c;
    char words[MAXLEN];
    while ((c = fgetc(f)) != EOF){
        if (c == ' ' || c == '\n') {
            if (count) {
                words[count] = '\0';
                addtree(h, words);
                sum++;
            }
            count = 0;
        }
        else{
            if (isalpha(c)){
                words[count] = c;
                count++;
            }
            else {
                if (count != 0) {
                    words[count] = '\0';
                    addtree(h, words);
                    sum++;
                }
                if (!isdigit(c)){
                    words[0] = c;
                    words[1] = '\0';
                    addtree(h, words);
                    sum++;
                }
                count = 0;
            }
        }
    }
    if (count) {
        words[count] = '\0';
        addtree(h, words);
        sum++;
    }
    count = 0;
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
        free(head);
    }
}
 
void freelist(struct list *l){
    if (l) {
        freelist(l->nextlist);
        free(l);
    }
}
 
void printtree(struct tree *head){
    if(head){
        printtree(head->left);
        printf("%s %d\n", head->arr, head->num);
        printtree(head->right);
    }
}
 
int main(int argc, char *argv[]) {
    int c;
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
    return 0;
}
