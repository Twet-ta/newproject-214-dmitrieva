#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include "serlib.h"

void connection(int cd, const socket_address& clAddr) {
    connected_socket_ cs(cd);
    string request;
    cs.read_(request);
    vector<string> lines = split_lines(request);
    string path = get_path(lines[0]);
    data_connection(cs, path, request);
}

void serv_loop() {
    socket_address servaddr;
    server_socket_ ss;
    ss.bind_(servaddr);
    ss.listen_(BACKLOG);
    for (;;) {
        socket_address claddr;
        int cd = ss.accept_(claddr);
        connection(cd, claddr);
    }
}

int main()
{
    serv_loop();
    return 0;
}
