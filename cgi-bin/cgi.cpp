#include <iostream> 
#include <stdlib.h>

int main () {
    //printf("Content-type: text/html\n\n");
    printf("<html>");
    printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    printf("<body>");
    printf("Приветствую! Вы ввели аргументы: ’%s’ с адреса ’%s’<br>", getenv("QUERY_STRING"), getenv("REMOTE_ADDR"));
    printf("Тип контента: ’%s’<br>", getenv("CONTENT_TYPE"));
    printf("Версия протокола CGI: ’%s’<br>", getenv("GATEWAY_INTERFACE"));
    printf("TCP-порт сервера: ’%s’<br>", getenv("SERVER_PORT"));
    printf("IP-адрес сервера: ’%s’<br>", getenv("SERVER_ADDR"));
    printf("</body></html>");
    return 0; 
}
