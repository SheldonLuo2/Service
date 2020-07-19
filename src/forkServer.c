#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>
#include <sys/wait.h>

#define SERV_PORT 8000
#define BUF_SIZE 1024

void wait_child(int signo)
{
    while(waitpid(0, NULL, WNOHANG) > 0);
    return;
}

int main()
{
    int sfd, cfd;
    int ret, cLen, n, i;
    char buf[BUF_SIZE], buf_ip[BUFSIZ];
    struct sockaddr_in serv_addr, client_addr;
    pid_t pid;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd  < 0)
    {
        perror("socket()");
        exit(-1);
    }
    //设置端口复用,在关闭2msl之前可以继续用该端口
    int flag = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0)
    {
        perror("bind()");
        exit(-1);
    }
    ret = listen(sfd, 128);
    if(ret < 0)
    {
        perror("listen()");
        exit(-1);
    }
    cLen = sizeof(client_addr);
    while(1)
    {
        cfd = accept(sfd, (struct sockaddr*)&client_addr, &cLen);
        if(cfd < 0)
        {
            perror("accept()");
            exit(-1);
        }
        printf("client IP:%s,port:%d\n", 
               inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, buf_ip, sizeof(buf_ip)),
                                                   ntohs(client_addr.sin_port));
        pid = fork();
        if(pid < 0)
        {
            perror("fork()");
            exit(-1);
        }
        else if(pid == 0)
        {
            close(sfd);
            break;
        }
        else
        {
            close(cfd);
            signal(SIGCHLD, wait_child);
        }
    }
    if(pid == 0)
    {
        while(1)
        {
            n = read(cfd, buf, sizeof(buf));
            if(n == 0)
            {
                close(cfd);
                exit(-1);
            }
            else if(n == -1)
            {
                perror("read()");
                exit(-1);
            }
            else
            {
                for(i = 0; i < n; i++)
                    buf[i] = toupper(buf[i]);
                write(cfd, buf, n);
                write(STDOUT_FILENO, buf, n);
            }
        }
    }


    close(sfd);
    exit(0);
}
