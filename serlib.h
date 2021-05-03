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
#include <sys/wait.h>
#include <errno.h>

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::string;
using std::to_string;

const int BACKLOG = 5; //queue
#define _PORT 8000
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
    void read_(vector<uint8_t>& bytes){
        char buf[_BUFF];
        if (recv(sd, buf, _BUFF, 0) < 0) cout << "Bad reading in connected socket" << endl;
        for (int i = 0; buf[i]; ++i) {
            bytes.push_back(buf[i]);
        }
    };
    int get_sd(){ return sd;}
};

class client_socket_ : public connected_socket_{
public:
    client_socket_() : connected_socket_() {}; //check
    void connect_(const socket_address& saddr){
        int len = sizeof(saddr);
        if(connect(sd, saddr.get_addr(), len) < 0) perror("Bad connecting with server") ;
    };
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
    temp.push_back(0);
    return temp;
}
void vect_to_file (int fd, const std::vector<uint8_t> &v) {
    for (uint8_t b : v) {
        write(fd, reinterpret_cast<const void *>(b), sizeof(b));
    }
}

void file_to_file (int fd1, int fd2) {
    char c;
    int k = 0;
    while(read(fd1, &c, 1) != EOF) {
        write(fd2, &c, 1);
        k++;
    }
}

char **get_env(const string& fname, const string& query) {
    char** env = new char*[7];
    env[0] = new char[14 + query.size()]; // QUERY_STRING
    strcpy(env[0], "QUERY_STRING=");
    strcat(env[0], query.c_str());
    env[1] = new char[22]; // REMOTE_ADDR
    env[1] = (char *) "REMOTE_ADDR=127.0.0.1";
    env[2] = new char[24]; // CONTENT_TYPE
    env[2] = (char *) "CONTENT_TYPE=text/plain"; // CONTENT_TYPE
    env[3] = new char[26]; // GATEWAY_INTERFACE
    env[3] = (char *) "GATEWAY_INTERFACE=CGI/1.1"; // GATEWAY_INTERFACE
    env[4] = new char[17]; // SERVER_PORT
    env[4] = (char *) "SERVER_PORT=8001"; // SERVER_PORT
    env[5] = new char[22]; // SERVER_ADDR
    env[5] = (char *) "SERVER_ADDR=127.0.0.1"; // SERVER_ADDR
    env[6] = nullptr;
    return env;
}

void data_connection(connected_socket_ cs, string path, const string& request) {
    int fd;
    if (path.find('?') == -1) {
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
     } else {
        int fd1;
        int status;
        pid_t pid;
        cout << "conn path: " << path << endl;
        if ((pid = fork()) < 0) perror("Bad fork");
        if (pid > 0) { // parent
            wait(&status);
            if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                fd1 = open("log.txt", O_RDONLY);
                vector<uint8_t> v = file_to_vect(fd1);
                cout << "Server sends: " << "HTTP/1.1 200 OK" << endl;
                cs.write_("HTTP/1.1 200 OK\0");
                string str;
                str += "\r\nVersion: HTTP/1.1\r\nContent-type: text/html\r\nContent-length: " + to_string(v.size()) + "\r\n\r\n";
                cout << "Version: " << "HTTP/1.1" << endl;
                cout << "Content-length: " << v.size() << endl;
                cs.write_(str);
                cs.write_(v);
                cs.shutdown_();
                close(fd1);
            }
        } else if (pid < 0) { perror("Disaster");
        } else { // child
            if ((fd1 = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) perror ("Not making file");
            dup2(fd1, 1);
            close(fd1);
            string fname;
            int i = 0;
            while(path[i] != '?') {
                fname += path[i];
                i++;
            }
            string vars;
            i = (int) fname.length() + 1;
            while(i < path.length()) {
                vars += path[i];
                i++;
            }
            char* argv[] = {(char*)fname.c_str(), nullptr};

            char** env = get_env(fname, vars);
            execve(fname.c_str(), argv, env);
            string str;
            switch (errno){
                case EACCES:
                    str = "HTTP/1.1 403 Forbidden\n";
                case ENETRESET:
                    str = "HTTP/1.1 503 Service Unavailable\n";
                case EADDRNOTAVAIL:
                    str = "HTTP/1.1 404 Not Found\n";
                default:
                    str = "HTTP/1.1 500 Internal Server Error \n";
            }
            cs.write_(str);
            cout << str << endl;
            perror("Bad exec");
            exit(1);
        }
    }
}

//_________________________________________


class http_reqest{
    //http_header_reqest r;
    //std::vector<http_header> http_h;
    //string body;
    string hr;
public:
    http_reqest() {
        //hr = "GET /cgi-bin/cgi?NAME=Tanya&SURNAME=D HTTP/1.1\r\0"; //check
        hr = "GET /index.html HTTP/1.1\r\0"; //check
    }
    string str() const {
        return hr;
    }
};


#endif //SERVER_SERLIB_H
