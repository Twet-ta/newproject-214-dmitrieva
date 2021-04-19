//
// Created by twetta on 16.04.2021.
//

#ifndef SERVER_SERLIB_H
#define SERVER_SERLIB_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <cstdint>
#include <fcntl.h>
using namespace std;

const int BACKLOG = 5; //queue
#define _PORT 8001
#define _BUFF 1024
#define _IP "127.0.0.1"

class socket_address{
    struct sockaddr_in saddr;
public:
    socket_address() {
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(_PORT);
        saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    };
    socket_address(const char* ip, short port){
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = inet_addr(ip);
    };
    socket_address(unsigned int ip, short port){
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = htonl(ip);
    };
    struct sockaddr* get_addr() const {return (sockaddr*)&saddr;};
    ~socket_address() {};
};

class socket_{
protected:
    int sd;
    explicit socket_(int sd_) : sd(sd_) {};
public:
    socket_(){
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1) cout << "Bad_creating" << endl;
    }
    void shutdown_() { shutdown(sd, 2); };
    ~socket_() {
        close(sd);
    }
};

class server_socket_ : public socket_{
public:
    server_socket_() : socket_(){}
    void listen_(int backlog){
        if (-1 == listen(sd, backlog)) {
            cout << "Bad listening" << endl;
        }
    }
    void bind_(const socket_address& ipaddr){
        int len = sizeof(ipaddr);
        if (0 != bind(sd, ipaddr.get_addr(), len))
        {
            perror("Bad binding");
            cout << endl;
            exit(0);
        }
    }
    int accept_(const socket_address& claddr){
        size_t len = sizeof(claddr);
        int a = accept(sd, claddr.get_addr(), (socklen_t*)&len);
        if (a < 0) cout << "Bad accepting" << endl;
        return a;
    }
};

vector<string> split_lines(string str){
    vector<string> s;
    string p = "\r\n";
    string str1;
    int pos = 0;
    int pos_s = 0;
    while((pos_s = str.find(p, pos)) > 0) {
        str1 = str.substr(pos, pos_s - pos);
        pos = pos_s + p.length();
        s.push_back(str1);
    }
    s.push_back(str.substr(pos));
    return s;
}

//vector<uint8_t> string_to_vector(string& str){
    //vector<uint8_t> v;
//}

class connected_socket_ : public socket_{ //stop
public:
    connected_socket_() = default;
    explicit connected_socket_(int sd) : socket_(sd) {};
    void write_(const string& str) {
        if (send(sd, str.c_str(), str.length(), 0) < 0) cout << "Bad writing in connected socket" << endl;
    };
    void write_(const vector<uint8_t>& bytes) {
        if (send(sd, bytes.data(), bytes.size(), 0) < 0) cout << "Bad writing in connected socket" << endl;
    };
    void read_(string& str){
        char buf[_BUFF];
        if (recv(sd, buf, _BUFF, 0) < 0) cout << "Bad reading in connected socket" << endl;
        str = buf;
    };
};

class client_socket_ : public connected_socket_{
public:
    void connect(const socket_address& servaddr);
};

string get_path(string str) {
    int i = 4;
    if (str[0] == 'H') i = 5;
    int pos = i;
    while (str[pos] != ' ') pos++;
    return str.substr(i + 1, pos - i - 1);
}

vector<uint8_t> file_to_vect(int fd){
    vector<uint8_t> temp;
    char c;
    while(read(fd, &c, 1)) temp.push_back(c);
    return temp;
}

void data_connection(connected_socket_ cs, string path) {
    int fd;
    cout << "conn path: " << path << endl;
    if (path.length() == 0) path = "index.html";
    if ((fd = open(path.c_str(), O_RDONLY)) < 0) {
        cs.write_("HTTP/1.1 404 Not Found\0");
        cout << "HTTP/1.1 404 Not Found" << endl;
        if ((fd = open("404.html", O_RDONLY)) < 0) {
            cout << "404 not found" << endl;
        }
    } else {
        cs.write_("HTTP/1.1 200 OK\0");
        cout << "HTTP/1.1 200 OK" << endl;
    }
    vector<uint8_t> v = file_to_vect(fd);
    string str = "\r\nVersion: HTTP/1.1\r\nContent-length: " + to_string(v.size()) + "\r\n\r\n";
    cout << "Version: " << "HTTP/1.1" << endl;
    cout << "Content-length: " << to_string(v.size()) << endl;
    cs.write_(str);
    cs.write_(v);
    cs.shutdown_();
    close(fd);
}
#endif //SERVER_SERLIB_H

/*
 *
 * struct dat{
    char Www[3];
    int dd;
    char Mmm[3];
    int YYYY;
    int hh;
    int mm;
    int ss;
};

struct date{
    dat d;
};

struct last_modified{
    dat d;
};

struct host{
    string name;
    int port;
};

struct refer{
    string uri;
};

struct user_agent{
    string name;
    int ch;
};

struct server{
    string name;
    int ch;
};

struct content_length{
    int num;
};

struct content_type{
    string type;
    string subtype;
};

struct allow{
    string method = "GET, HEAD";
};
 void client_connection(){
    client_socket_ s;
    socket_address saddr("127.0.0.1", 1555);
    s.connect(saddr);
    //make request
    http_request rq;
    //make request ...
    s.write_(rq.to_string());
    string str_resp;
    s.read_(str_resp);
    http_response resp;
    vector<string> lines = split_;ines(str_resp);
    // parse responce ...;
    s.shutdown();
}

class http_header{
    string name;
    string value;
public:
    http_header(const string& n, const string& v) : name(n), value(v) {};
    static http_header* parse_header(const string& line);
    //explicit http_header(const string& line); // server <-> reqest_header
    //string to_string() const{...};
};

class http_header_main : http_header{
    date d;
};

class http_header_essence : http_header{
    content_length cl;
    content_type ct;
    allow a;
    last_modified lm;
};

class http_header_reqest : http_header{
    host h;
    refer r;
    user_agent ua;
};

class http_header_response : http_header{
    server s;
};

class http_reqest{
    http_header_reqest r;
    std::vector<http_header> http_h;
    string body;
};

class http_response_head{
    http_header_response r;
    std::vector<http_header> http_h;
};

class http_response_get : http_response_head{
    string body;
};

class http_server{
public:
    http_response_get GET(http_header_reqest& h){};
    http_response_head HEAD(http_header_reqest& h){};
};

class http_client{
};

//<схема>://<хост><путь к файлу-ресурсу>[?<параметры запроса>] - URI

*/