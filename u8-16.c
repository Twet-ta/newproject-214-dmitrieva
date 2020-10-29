#include <stdlib.h>
#include <stdio.h>


void error(int offset) {
    fprintf(stderr, "Error with %d byte\n", offset);
}

unsigned short reverse(unsigned short x) {
    unsigned short y = x & 0b11111111;
    x >>= 8;
    x = x | (y << 8);
    return x;
}

unsigned short read16(FILE *fin, int *offset, int *flag) {
    unsigned short ch16 = 0;
    unsigned char ch8;
    if (!fread(&ch8, sizeof(ch8), 1, fin)) {
        *flag = 0;
        return 0;
    }
    (*offset)++;
    if (ch8 >> 7 == 0b0) {
        ch16 = ch8;
    } else if (ch8 >> 5 == 0b110) {
        ch16 = (ch8 & 0b11111) << 6;
        if (!fread(&ch8, sizeof(ch8), 1, fin)) {
            fprintf(stderr, "Error with 2-nd byte in symbol, it isn't exist\n");
            error(*offset);
            *flag = 0;
            return 0;
        }
        (*offset)++;
        if (ch8 >> 6 == 0b10) {
            ch16 = ch16 | (ch8 & 0b111111);
        } else {
            fprintf(stderr, "Error with 2-nd byte in symbol, its value: %x\n", ch8);
            error(*offset);
        }
    } else if (ch8 >> 5 == 0b111) {
        ch16 = (ch8 & 0b1111) << 12;
        if (!fread(&ch8, sizeof(ch8), 1, fin)) {
            fprintf(stderr, "Error with 2-nd byte in symbol, it isn't exist\n");
            error(*offset);
            *flag = 0;
            return 0;
        }
        (*offset)++;
        if ((ch8 >> 6) == 0b10) {
            ch16 = ch16 | ((ch8 & 0b111111) << 6);
            if (!fread(&ch8, sizeof(ch8), 1, fin)) {
                fprintf(stderr, "Error with 3-nd byte in symbol, it isn't exist\n");
                error(*offset);
                *flag = 0;
                return 0;
            }
            (*offset)++;
            if (ch8 >> 6 == 0b10) {
                ch16 = ch16 | (ch8 & 0b111111);
            } else {
                fprintf(stderr, "Error with 3-nd byte in symbol, its value: %x\n", ch8);
                error(*offset);
            }
        } else {
            fprintf(stderr, "Error with 2-nd byte in symbol, its value: %x\n", ch8);
            error(*offset);
        }
    } else {
        fprintf(stderr, "Error with 1-st byte in symbol, its value: %x\n", ch8);
        error(*offset);
    }
    return ch16;
}

int main (int argc, char *argv[])
{
    FILE *fin = stdin;
    FILE *fout = stdout;
    int offset = 0, flag = 1, bom = 0; 
    unsigned short ch16;
    if (argc == 2) {
        fin = fopen(argv[1], "r");
    } else if (argc == 3) {
        fin = fopen(argv[1], "r");
        fout = fopen(argv[2], "w");
    }
    if (!(fin && fout)) {
        fprintf(stderr, "File doesn't exist\n");
        exit(1);
    }
    ch16 = read16(fin, &offset, &flag);
    if (ch16 == 0xFFFE) {
        bom = 0;
    } else if (ch16 == 0xFEFF) {
        bom = 1;
    } else {
        bom = 1;
        if (flag) {
            fwrite(&ch16, sizeof(ch16), 1, fout);
        }
    }
    while (flag) {
        ch16 = read16(fin, &offset, &flag);
        if (flag) {
            if (bom) {
                fwrite(&ch16, sizeof(ch16), 1, fout);
            } else {
                ch16 = reverse(ch16);
                fwrite(&ch16, sizeof(ch16), 1, fout);
            }
        }
    }
    return 0;
}
