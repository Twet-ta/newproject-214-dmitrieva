#include <iostream>
#include <cstdlib>
#include <string>

enum TypeOfCover{
    SOFT,
    HARD
};

enum Color{
    BW,
    CL,
};

class catalog{
protected:
    int size;
    int num;
    std::string *s;
    static int number; //number of catalogs
    static int numberofbooks; //number of books
public:
    virtual int CostOfPrint() const = 0;
    virtual int CostOfPrint(const int i) const = 0;
    void resize(){
        std::string *t;
        t = new std::string [size*2 + 1];
        for(int i = 0; i < size; ++i){
            t[i] = s[i];
        }
        delete [] s;
        s = t;
        size = size*2 + 1;
    }
    void resize(const int i){
        if (size < i) {
            std::string *t;
            t = new std::string[i];
            for (int k = 0; k < i; ++k) {
                t[k] = s[k];
            }
            delete[] s;
            s = t;
            size = i;
        }
    }
    catalog(){
        size = 1;
        number++;
        num = 0;
        s = new std::string[size];
    }
    catalog(const int v){
        size = v;
        number++;
        num = 0;
        s = new std::string[size];
    }
    catalog(const catalog& t){
        size = t.size;
        s = new std::string [size];
        number++;
        num = t.num;
        for(int i = 0; i < size; ++i){
            s[i] = t.s[i];
        }
    }
    catalog& operator= (const catalog& t)
    {
        resize(t.size);
        for(int i = 0; i < size; ++i){
            s[i] = t.s[i];
        }
        num = t.num;
        return *this;
    }
    catalog& operator= (const int t)
    {
        if (size < t) resize(t);
        return *this;
    }
    virtual ~catalog(){
        delete [] s;
        number--;
        numberofbooks -=num;
    }
    std::string Name(const int id) const{
        return s[id];
    }
    void PrintCatalog() const{
        for(int i = 0; i < num; ++i){
            std::cout << s[i] << " ";
        }
        std::cout << std::endl;
    }
    void PrintCatalog(const int n) const{
        if (n >= num) std::cout << "Wrong number of elements in catalog";
        else {
            for (int i = 0; i < n; ++i) {
                std::cout << s[i] << " ";
            }
        }
        std::cout << std::endl;
    }
    void PrintInfo() const{
        PrintCatalog();
        std::cout << "Cost of print:";
        int sum = CostOfPrint();
        std::cout << sum << std::endl;
    }
    void PrintInfo(const int i) const{
        PrintCatalog(i);
        std::cout << "Cost of print:";
        int sum = CostOfPrint(i);
        std::cout << sum << std::endl;
    }
    std::string operator()(const int index){
        if (index >= num) {
            std::cout << "Wrong index";
            return "";
        }
        else return s[index];
    }
    std::string operator()(const int index, const int i){
        if (index >= num) {
            std::cout << "Wrong index";
            return "";
        }
        if (i >= s[index].length()) {
            std::cout << "Wrong length";
            return "";
        }
        else return s[index].substr(i, 1);
    }
    void Add(const std::string t){
        num++;
        numberofbooks++;
        if (num >= size) resize();
        s[num-1] = t;
    }
    void Delete(){
        num--;
        numberofbooks++;
    }
    const int catalogs(){
        return number;
    }
    const int books(){
        return num;
    }
    const int booksincatalog(){
        return numberofbooks;
    }
};

class book : public catalog{
    TypeOfCover toc;
    bool IsDustJacket;
public:
    book() : catalog() {
        toc = SOFT;
        IsDustJacket = 0;
    }
    book(const int i) : catalog(i){
        toc = SOFT;
        IsDustJacket = 0;
    }
    book(const int i, const int j) : catalog(){
        toc = static_cast<TypeOfCover>(i%2);
        IsDustJacket = j;
    }
    int CostOfPrint() const{
        return (toc*5 + (!toc)*3 + IsDustJacket*2) * num;
    };
    int CostOfPrint(const int i) const{
        return i * (toc*5 + (!toc)*3 + IsDustJacket*2);
    };
    book (const book& t) : catalog(t){
        toc = t.toc;
        IsDustJacket = t.IsDustJacket;
    }
    book& operator= (const book& t) {
        resize(t.size);
        for(int i = 0; i < size; ++i){
            s[i] = t.s[i];
        }
        num = t.num;
        toc = t.toc;
        IsDustJacket = t.IsDustJacket;
        return *this;
    }
    book& operator= (const int t) {
        if (size < t) resize(t);
        return *this;
    }
};

class brochure : public catalog{
    Color br;
    bool HasPics;
public:
    brochure() : catalog() {
        br = BW;
        HasPics = 0;
    }
    brochure(const int i) : catalog(i){
        br = BW;
        HasPics = 0;
    }
    brochure(const int i, const int j) : catalog(){
        br= static_cast<Color>(i%2);
        HasPics = j;
    }
    int CostOfPrint() const{
        return num * (br*4 + (!br)*2 + HasPics*10);
    };
    int CostOfPrint(const int i) const{
        return i * (br*4 + (!br)*2 + HasPics*10);
    };
    brochure (const brochure& t) : catalog(t){
        br = t.br;
        HasPics = t.HasPics;
    }
    brochure& operator= (const brochure& t) {
        resize(t.size);
        for(int i = 0; i < size; ++i){
            s[i] = t.s[i];
        }
        num = t.num;
        br = t.br;
        HasPics = t.HasPics;
        return *this;
    }
    brochure& operator= (const int t) {
        if (size < t) resize(t);
        return *this;
    }
};

class leaflet : public catalog{
    int sides;
    Color br;
public:
    leaflet(): catalog(){
        br = BW;
        sides = 0;
    }
    leaflet(const int i) : catalog(i){
        br = BW;
        sides = 0;
    }
    leaflet(const int i, const int j) : catalog(){
        br = static_cast<Color>(i%2);
        sides = j%2;
    }
    int CostOfPrint() const{
        return num * (br*4 + (!br)*2 + sides*3);
    };
    int CostOfPrint(const int i) const{
        return i * (br*4 + (!br)*2 + sides*3);
    };
    leaflet (const leaflet& t) : catalog(t){
        br = t.br;
        sides = t.sides;
    }
    leaflet& operator= (const leaflet& t) {
        resize(t.size);
        for(int i = 0; i < size; ++i){
            s[i] = t.s[i];
        }
        num = t.num;
        br = t.br;
        sides = t.sides;
        return *this;
    }
    leaflet& operator= (const int t) {
        if (size < t) resize(t);
        return *this;
    }
};

int catalog :: number = 0;
int catalog :: numberofbooks = 0;

int main() {
    book b1(2);
    brochure b2(1,1);
    leaflet b3(0, 1);
    b1.Add("Peace and War");
    b1.Add("Peace");
    b1.Add("War");
    b1.Add("War1");
    b1.PrintCatalog();
    b1.PrintCatalog(1);
    std::cout << b1(1) << std::endl;
    std::cout << b1.catalogs() << std::endl;
    std::cout << b1.books() << std::endl;
    b1.PrintInfo();
    b2.Add("2Peace and War");
    b2.Add("2Peace");
    b2.Add("2War");
    b2.Add("2War1");
    b2.PrintCatalog();
    b2.PrintCatalog(1);
    std::cout << b2(1) << std::endl;
    std::cout << b2.catalogs() << std::endl;
    std::cout << b2.books() << std::endl;
    b2.PrintInfo();
    b2.PrintInfo(1);
    b2.PrintInfo(2);
    b1.PrintInfo(1);
    b1.PrintInfo(2);
    std::cout << b2.booksincatalog() << std::endl;
    return 0;
}

