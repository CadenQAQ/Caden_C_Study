#include <sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>


void *client_thread (void *arg)
{
    int clientfd = *(int *)arg;

    while(1)
    {
    char buffer[128] = {0};
    int count = recv(clientfd, buffer, 128, 0); //return the length of the received data

    if (count == 0)
    {
        break;
    }

    send(clientfd, buffer, count, 0);
    printf("clientfd: %d, count: %d, buffer: %s\n" , clientfd, count, buffer);
    }

    close(clientfd);

}

int main()

    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(struct sockaddr_in));

        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(2048);

        if (-1 == bind(sockfd,(struct sockaddr*)&serveraddr, sizeof(struct sockaddr)))
        {
            perror("bind");
            return -1;
        }

        listen(sockfd, 10);


        while (1)
        {
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr, &len);//阻塞的函数，一直等待有连接

        pthread_t thid;
        pthread_create(&thid, NULL, client_thread, &clientfd);
        }
        //sockfd is 3, clientfd is 4, 
        //stdin is 0, stdout is 1, stderr is 2

        getchar();
        

    }
