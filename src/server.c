#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"

int main()
{
    int lfd;
    int cfd;
    int n, i;
    char buf[BUFSIZ];
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addrlen;
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(lfd, 128);
    client_addrlen = sizeof(client_addr);
    cfd = accept(lfd, (struct sockaddr*)&client_addr, &client_addrlen);
    while(1)
    {
        n = read(cfd, buf, sizeof(buf));
        if(buf == "exit")
            break;
        for(i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);
        write(cfd, buf, n);
    }
    close(lfd);
    close(cfd);
    exit(0);
}

