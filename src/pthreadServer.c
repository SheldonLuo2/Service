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
#include <pthread.h>

#define SERV_PORT 8000
#define BUF_SIZE 1024

typedef struct sockInfo
{
    pthread_t id;
    int fd;
    struct sockaddr_in addr;
}Sockinfo;

void* worker(void* arg)
{
    char ip[64];
    char buf[1024];
    int i = 0;
    Sockinfo *info = (Sockinfo*)arg;
    while(1)
    {
        printf("Client IP:%s, port:%d\n", inet_ntop(AF_INET, &info->addr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(info->addr.sin_port));

        int len = read(info->fd, buf, sizeof(buf));
        if(len < 0)
        {
            perror("read()");
            pthread_exit(NULL);
        }
        else if(len == 0)
        {
            fprintf(stdout, "客户端断开连接...\n");
            close(info->fd);
            break;
        }
        else
        {
            printf("recv buf:%s\n", buf);
            for(i = 0; i < len; i++)                                                                                                                                                                
                buf[i] = toupper(buf[i]);
            write(info->fd, buf, sizeof(buf));
            write(STDOUT_FILENO, buf, len);

        }
    }
    return NULL;
}

int main()
{
    int sfd;
    int i = 0;
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0)
    {
        perror("socket()");
        exit(-1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind()");
        exit(-1);
    }
    if(listen(sfd, 128) < 0)
    {
        perror("listen");
        exit(-1);
    }
    printf("Strating Connect...\n");
    Sockinfo info[256];
    socklen_t cli_len = sizeof(struct sockaddr_in);
    for(i = 0; i < sizeof(info)/sizeof(info[0]); ++i)
    {
        info[i].fd = -1;
    }
    while(1)
    {
        for(i = 0; i < 256; ++i)
        {
            if(info[i].fd == -1)
            {
                break;
            }
        }
        if(i == 256)
        {
            break;
        }
        //主线程-等待接收连接请求
        info[i].fd = accept(sfd, (struct sockaddr*)&info->addr, &cli_len);
        if(info[i].fd < 0)
        {
            perror("accept()");
            exit(-1);
        }
        pthread_create(&info[i].id, NULL, worker, &info[i]);
        //设置线程分离
        pthread_detach(info[i].id);
    }

    close(sfd);
    pthread_exit(NULL);
    return 0;
}
