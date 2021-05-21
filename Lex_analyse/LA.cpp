#include <string>
#include <cctype>
#include <iostream>
#include <vector>
#include <algorithm>

enum type_of_lex {
    LEX_NULL,                                                                                   /* 0*/
    LEX_BOOL, LEX_DO, LEX_ELSE, LEX_IF, LEX_FALSE, LEX_INT,                                     /* 6*/
    LEX_NOT, LEX_OR, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR, LEX_WHILE, LEX_WRITE,               /*14*/
    LEX_BREAK,  LEX_CONTINUE, LEX_FOR, LEX_FUNC, LEX_IN, LEX_OBJECT, LEX_RETURN,                /*21*/
    LEX_STR, LEX_NUMB, LEX_IDENT,                                                               /*24*/

    LEX_EMPTY, LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_DOT, LEX_LPAREN, LEX_RPAREN,            /*31*/
    LEX_LCURPAREN, LEX_RCURPAREN, LEX_LSQPAREN, LEX_RSQPAREN, LEX_EQ, LEX_EQ2, LEX_EQ3,         /*38*/
    LEX_LESS, LEX_MORE, LEX_PLUS, LEX_EQPLUS, LEX_PLUS2, LEX_MINUS2, LEX_EQMINUS,               /*45*/
    LEX_MINUS, LEX_MUL, LEX_EQMUL, LEX_EQSLSH, LEX_SLSH, LEX_PRCNT, LEX_EQPRCNT,                /*52*/
    LEX_EQLESS, LEX_EQMORE, LEX_EQSIGN, LEX_SIGN, LEX_EQ2SIGN, LEX_AMP, LEX_AMP2, LEX_PIPE,     /*60*/
    LEX_PIPE2,                                                                                  /*61*/
};

enum state{S, IDENT, NUMB, COM, ALE, EQ, STR, FIN, PLUS, MINUS, AMP, PIPE, EQ2, SLSH, COM2, CHECK};

const char *TW    [] = { "", "boolean", "do", "else", "if", "false", "int", "not", "or",
                              "read", "then", "true", "var", "while", "write", "break", "continue",
                              "for", "function", "in", "Object", "return", "string", "number", "id", NULL };

const char *TD    [] = { "", ";", ",", ":", ".", "(", ")", "{", "}", "[", "]", "=", "==", "===",
                         "<", ">", "+", "+=", "++", "--", "-=", "-", "*", "*=", "/=", "/",
                         "%", "%=", "<=",  ">=","!=", "!", "!==", "&", "&&", "|", "||", NULL };

struct Token {
    type_of_lex type;
    std::string s;
    int n;
    int ntd;//position_in_TD;
    int ntw;//position_in_TW;
    int ntid;//position_in_TID;
    Token(type_of_lex t = LEX_NULL, std::string s1 = "", int m = 0, int k = 0, int l = 0, int r = 0) :
        type(t), s(s1), n(m), ntd(k), ntw(l), ntid(r) {};
    void Clear() { type = LEX_NULL; n = 0; s.clear(); ntd = 0; ntw = 0, ntid = 0;}
    bool operator==(const std::string& s1){return (s == s1);}
};

std::vector<Token> TID;

int PUT( const std::string &buf ) {
    std::vector<Token>::iterator k;
    if((k = std::find(TID.begin(), TID.end(), buf)) != TID.end())
        return k - TID.begin();
    TID.push_back(Token(LEX_IDENT, buf, 0, 0, 0, TID.size() - 1));
    return TID.size() - 1;
}

int look ( const std::string buf, const char ** list ) {
    int i = 0;
    while ( list[i] ) {
        if ( buf == list[i] )
            return i;
        ++i;
    }
    return 0;
}

class Scanner {
protected:
    int ch;
    int j;
private:
    Token curToken;
    state curState;
    virtual void gc() = 0;
    void step() {
        switch (curState) {
            case S:
                if (isspace(ch)) gc();
                else if (ch == '"') {
                    curState = STR; curToken.type = LEX_STR; gc();
                }
                else if (std::isdigit(ch)) {
                    curState = NUMB; curToken.type = LEX_NUMB; curToken.n = ch - '0'; gc();
                }
                else if (std::isalpha(ch)) {
                    curState = IDENT; curToken.type = LEX_IDENT; curToken.s.push_back(ch); gc();
                }
                else if ( ch == '*' || ch == '<' || ch == '>' || ch == '%') {
                    curToken.s.push_back (ch);
                    curState  = ALE;
                    gc();
                }
                else if ( ch == '+') {
                    curToken.s.push_back (ch);
                    curState  = PLUS;
                    gc();
                }
                else if ( ch == '-') {
                    curToken.s.push_back (ch);
                    curState  = MINUS;
                    gc();
                }
                else if ( ch == '&') {
                    curToken.s.push_back (ch);
                    curState  = AMP;
                    gc();
                }
                else if ( ch == '|') {
                    curToken.s.push_back (ch);
                    curState  = PIPE;
                    gc();
                }
                else if (ch == '!' || ch == '=') {
                    curToken.s.push_back (ch);
                    curState  = EQ;
                    gc();
                }
                else if ( ch == '#') {
                    curToken.s.push_back (ch);
                    curState  = COM;
                    gc();
                }
                else if ( ch == '/') {
                    curToken.s.push_back (ch);
                    curState  = SLSH;
                    gc();
                }
                else if (ch == '@'){ //clion
                    curState = FIN;
                    curToken.type = LEX_NULL;
                }
                else if (ch == EOF){ //file
                    curState = FIN;
                    curToken.type = LEX_NULL;
                }
                else {
                    curToken.s.push_back(ch);
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                        gc();
                    }
                    else
                        throw "not found this symbol";
                }
                break;
            case NUMB:
                if (isdigit(ch)) {
                    curToken.n = curToken.n * 10 + ( ch - '0' );
                    gc();
                    curState = NUMB;
                }
                else {
                    curToken.type = LEX_NUMB;
                    curState = FIN;
                }
                break;
            case PLUS:
                if ( ch == '=' || ch == '+') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case MINUS:
                if ( ch == '=' || ch == '-') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case AMP:
                if ( ch == '&') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case SLSH:
                if ( ch == '*') {
                    gc();
                    curState = COM2;
                }
                else if (ch == '='){
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case COM2:
                if ( ch == '*') {
                    gc();
                    curState = CHECK;
                }
                else {
                    curState = COM2;
                    gc();
                }
                break;
            case CHECK:
                if ( ch == '/') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    curState = COM2;
                    gc();
                }
                break;
            case PIPE:
                if ( ch == '|') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case COM:
                if ( ch == '\n') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    curState = COM;
                    gc();
                }
                break;
            case EQ:
                if ( ch == '=') {
                    curToken.s.push_back ( ch );
                    gc();
                    curState = EQ2;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case EQ2:
                if ( ch == '=') {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    if ((j = look(curToken.s, TD))){
                        curState = FIN;
                        curToken.type = (type_of_lex) ( j + (int)LEX_EMPTY);
                        curToken.ntd = j;
                    }
                }
                break;
            case ALE:
                if ( ch == '=' ) {
                    curToken.s.push_back ( ch );
                    j = look( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    gc();
                    curState = FIN;
                }
                else {
                    j   = look ( curToken.s, TD );
                    curToken.type = (type_of_lex) ( j + (int) LEX_EMPTY);
                    curToken.ntd = j;
                    curState = FIN;
                }
                break;
            case IDENT:
                if (isdigit(ch) || isalpha(ch) || (ch == '_')) {
                    curToken.s.push_back(ch);
                    curState = IDENT;
                    gc();
                } else {
                    if (j = look (curToken.s, TW)) {
                        curToken.type = (type_of_lex)j;
                        curToken.ntw = j;
                    } else {
                        j = PUT(curToken.s);
                        curToken = LEX_IDENT;
                        curToken.ntid = j;
                    }
                    curState = FIN;
                }
                break;
            case STR:
                if (ch == '"') {
                    curToken.type = LEX_STR;
                    j = look("string", TW );
                    curToken.type = (type_of_lex)j;
                    curToken.ntw = j;
                    gc();
                    curState = FIN;
                } else if (std::cin.eof()) throw ch;
                else {
                    curToken.s.push_back(ch);
                    curState = STR;
                    gc();
                }
                break;
            case FIN:
                throw ("EOF problems");
                break;
        }
    }
public:
    Scanner() : ch(' '), curState(S) {};
    Token GetToken(){
        curToken.Clear();
        curState = S;
        while (ch == ' ') gc();
        while (curState != FIN)
            step();
        return curToken;
    }
    Token PeekToken() {return curToken;}
};

class StreamScanner : public Scanner {
    FILE * fIn;
public:
    StreamScanner(FILE * stream = stdin) : fIn(stream) {}
    void gc() override{
        ch = fgetc(fIn);
    }
public:
};

class StringScanner : public Scanner {
    std::string str;
    size_t index;
public:
    StringScanner(const char * input) : str(input), index(0) {};
    void gc() override{
        if (index >= str.size())
            ch = EOF;
        else
            ch = str[index++];
    }
public:
};


int main (int argc, char ** argv) {
    Scanner* ps;
    if (argc == 1)
        ps = new StreamScanner(stdin);
    else
        ps = new StringScanner(argv[1]);
    Token token;
    try {
        while (ps->GetToken().type != LEX_NULL) {
            std::cout << ps->PeekToken().type << " ";
            if (ps->PeekToken().type == LEX_STR)  std::cout << "string " << ps->PeekToken().s;
            else if (ps->PeekToken().type == LEX_NUMB)  std::cout << "number " << ps->PeekToken().n;
            else if (ps->PeekToken().type == LEX_IDENT)  std::cout << "TID " << TID[ps->PeekToken().ntid].s<< " number in TID " << ps->PeekToken().ntid;
            else if (ps->PeekToken().ntd)  std::cout << "TD " << TD[ps->PeekToken().ntd];
            else if (ps->PeekToken().ntw)  std::cout << "TW " << TW[ps->PeekToken().ntw];
            std::cout << std::endl;
        }
    }
    catch (const char* msg){
        std::cout << "Error" << msg << std::endl;
    }
    delete ps;
}
