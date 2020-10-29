#include <stdlib.h>
#include <stdio.h>

unsigned short reverse(unsigned short x) {
    unsigned short y = x & 0b11111111;
    x >>= 8;
    x = x | (y << 8);
    return x;
}

unsigned short read16(FILE *fin, int *flag)
{
    unsigned short ch16 = 0;
    unsigned char ch8;
    if (!fread(&ch8, sizeof(ch8), 1, fin)) {
        *flag = 0;
        return 0;
    }
    ch16 = ch16 | ch8;
    if (!fread(&ch8, sizeof(ch8), 1, fin)) {
        fprintf(stderr, "Odd number of bytes\n");
        *flag = 0;
        return 0;
    }
    ch16 |= (ch8 << 8);
    return ch16;
}

void write_bytes(FILE *fout, unsigned short ch16)
{
    unsigned char ch8;
    if (ch16 <= 127) {
        ch8 = (unsigned char) (ch16 & 0xFF);
        fwrite(&ch8, sizeof(ch8), 1, fout);
    } else if (ch16 <= 2047) {
        ch8 = (ch16 & (0b11111 << 6)) >> 6;
        ch8 |= 0b11000000;
        fwrite(&ch8, sizeof(ch8), 1, fout);
        ch8 = ch16 & (0b111111);
        ch8 |= 0b10000000;
        fwrite(&ch8, sizeof(ch8), 1, fout);
    } else if (ch16 <= 65535) {
        ch8 = (ch16 & (0b1111 << 12)) >> 12;
        ch8 |= 0b11100000;
        fwrite(&ch8, sizeof(ch8), 1, fout);
        ch8 = (ch16 & (0b111111 << 6)) >> 6;
        ch8 |= 0b10000000;
        fwrite(&ch8, sizeof(ch8), 1, fout);
        ch8 = ch16 & (0b111111);
        ch8 |= 0b10000000;
        fwrite(&ch8, sizeof(ch8), 1, fout);
    }
}

int main (int argc, char *argv[])
{
    FILE *fin = stdin;
    FILE *fout = stdout;
    int flag = 1, bom = 0;
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
    ch16 = read16(fin, &flag);
    if (ch16 == 0xFFFE) {
        bom = 0;
    } else if (ch16 == 0xFEFF) {
        bom = 1;
    } else {
        bom = 1;
        if (flag) {
            write_bytes(fout, ch16);
        }
    }
    while (flag) {
        ch16 = read16(fin, &flag);
        if (flag) {
            if (!bom) {
                ch16 = reverse(ch16);
                write_bytes(fout, ch16);
            } else {
                write_bytes(fout, ch16);
            }
        }
    }
    return 0;
}
