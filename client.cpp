//
// Created by twetta on 02.05.2021.
//
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include "/home/twetta/CLionProjects/server/serlib.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
using namespace std;

int main()
{
    socket_address saddr;
    client_socket_ cs;
    cs.connect_(saddr);
    http_reqest hr;
    cs.write_(hr.str());
    file_to_file(cs.get_sd(),1);
    cs.shutdown_();
    return 0;
}

/*
 * Примеры вводимых строк:
 * 1)
 * 2)
 */
