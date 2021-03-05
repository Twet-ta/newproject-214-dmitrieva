#include <iostream>
#include <cstdlib>

class vector{
    int *p;
    int bord1;
    int bord2;
public:
    vector(){ //constr 0
        std::cout << "Enter borders" << std::endl;
        std::cin >> bord1 >> bord2;
        if (bord1 >= bord2) {
            std::cout << "Bad borders" << std::endl;
            bord1 = 0;
            bord2 = 1;
            p = new int[1];
            *p = 1;
        } else {
            std::cout << "Enter coordinates" << std::endl;
            p = new int[bord2 - bord1];
            for (int i = 0; i < bord2 - bord1; i++) {
                std::cin >> p[i];
            }
        }
    }
    vector(int a, int b){ //constr 1
        if (a >= b){
            std::cout << "Bad borders" << std::endl;
            bord1 = 0;
            bord2 = 1;
            p = new int[1];
            *p = 1;
        } else {
            p = new int[b - a];
            bord1 = a;
            bord2 = b;
            for (int i = 0; i < b - a; i++) {
                p[i] = rand() % 10;
            }
        }
    }
    vector(const vector& t){ //copy
        p = new int[t.bord2-t.bord1];
        bord1 = t.bord1;
        bord2 = t.bord2;
        for(int i = 0; i < t.bord2-t.bord1; ++i) {
            p[i] = t.p[i];
        }
    }
    void set(int index, int value){p[index] = value;}
    int get(int index) const { return p[index];}
    friend vector operator*(const vector&, const int);
    int operator[](const int index){
        if (index >= bord2) {
            std::cout << "Bad index" << std::endl;
            return p[bord1];
        }
        return p[index - bord1];
    }
    vector operator+(vector& b) const{
        if (!((bord1 == b.bord1) && (bord2 == b.bord2))) {
            std::cout << "Bad borders" << std::endl;
            return b;
        }
        vector sum(b.bord1, b.bord2);
        for (int i = 0; i < b.bord2 - b.bord1; i++) {
            sum.set(i, get(i) + b.get(i));
        }
        return sum;
    }
    vector operator-(vector& b){
        if (!((bord1 == b.bord1) && (bord2 == b.bord2))) {
            std::cout << "Bad borders" << std::endl;
            return b;
        }
        vector dif(b.bord1, b.bord2);
        for (int i = 0; i < b.bord2 - b.bord1; i++) {
            dif.set(i, get(i) - b.get(i));
        }
        return dif;
    }
    vector operator/(const int a){
        vector y(bord1, bord2);
        for (int i = 0; i < bord2 - bord1; i++) {
            y.set(i, get(i) / a);
        }
        return y;
    }
    void printelem(const int index){
        if (index >= bord2) {
            std::cout << "Bad index" << std::endl;
        } else {
            std::cout << p[index - bord1] << std::endl;
        }
    }
    void printarr(){
        for (int i = 0; i < bord2 - bord1; i++) {
            std::cout << p[i] << ' ';
        }
        std::cout << std::endl;
    }
    ~vector() { //destr
        delete [] p;
    };
};

vector operator*(const vector& x, const int a){
    vector y(x.bord1, x.bord2);
    for (int i = 0; i < x.bord2 - x.bord1; i++) {
        y.set(i, x.get(i) * a);
    }
    return y;
}

int main() {
    int r;
    vector *x = new vector(0,3);
    vector *y = new vector(0,3);
    std::cout << "MENU" << std::endl;
    while(1) {
        std::cout << "Referring to a single element? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector:" << std::endl;
            (*x).printarr();
            std::cout << "Enter index" << std::endl;
            int i;
            std::cin >> i;
            std::cout << "Your element:" << (*x)[i] << std::endl;
        }
        std::cout << "Creating an object with arbitrary borders? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "1 - create new vector x, 0 - choose only borders" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            } else {
                int b1, b2;
                std::cin >> b1 >> b2;
                delete x;
                vector *x = new vector(b1, b2);
            }
            std::cout << "Your vector:" << std::endl;
            (*x).printarr();
            std::cout << "Enter index" << std::endl;
            int i;
            std::cin >> i;
            (*x).printelem(i);
        }
        std::cout << "Addition? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
            std::cout << "Create new vector y? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete y;
                vector *y = new vector;
            }
            std::cout << "Your vector y:" << std::endl;
            (*y).printarr();
            std::cout << "Their add:" << std::endl;
            (*x + *y).printarr();
        }
        std::cout << "Subtraction? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
            std::cout << "Create new vector y? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete y;
                vector *y = new vector;
            }
            std::cout << "Your vector y:" << std::endl;
            (*y).printarr();
            std::cout << "Their sub:" << std::endl;
            (*x - *y).printarr();
        }
        std::cout << "Multiplication? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
            std::cout << "Enter number a? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            int a = 5;
            if (r == 1) {
                r = 0;
                std::cin >> a;
            }
            std::cout << "Their mul:" << std::endl;
            (*x * a).printarr();
        }
        std::cout << "Division? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
            std::cout << "Enter number a? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            int a = 5;
            if (r == 1) {
                r = 0;
                std::cin >> a;
            }
            std::cout << "Their div:" << std::endl;
            (*x/a).printarr();
        }
        std::cout << "Element output? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
            std::cout << "Enter index" << std::endl;
            int i;
            std::cin >> i;
            (*x).printelem(i);
        }
        std::cout << "Array output? 1 - YES, 0 - NO" << std::endl;
        std::cin >> r;
        if (r == 1) {
            r = 0;
            std::cout << "Create new vector x? 1 - YES, 0 - NO" << std::endl;
            std::cin >> r;
            if (r == 1) {
                r = 0;
                delete x;
                vector *x = new vector;
            }
            std::cout << "Your vector x:" << std::endl;
            (*x).printarr();
        }
        std::cout << "Repeat MENU? 1 - YES, 0 - EXIT" << std::endl;
        std::cin >> r;
        if (r == 0) {
            return 0;
        }
    }
}
