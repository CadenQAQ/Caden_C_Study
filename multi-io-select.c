#include <sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>

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


        fd_set rfds, rset;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);

        int maxfd = sockfd;
        printf("loop\n");

        while(1){
            rset = rfds;

            int nready = select(maxfd+1, &rset, NULL, NULL, NULL); //nready是事件个数,即连接个数

            if(FD_ISSET(sockfd, &rset))
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr, &len);
                printf("sockfd: %d\n", clientfd);

                FD_SET(clientfd, &rfds);

                maxfd = clientfd;
            }

            int i = 0;
            for (i = sockfd +1; i<=maxfd; i++)
            {
                if(FD_ISSET(i, &rset))
                {
                    char buffer[128] = {0};
                    int count = recv(i, buffer, 128, 0);

                    if(count ==0)
                    {
                        printf("disconnected\n");

                        FD_CLR(i, &rfds);
                        close(i);

                        break;
                    }

                    send(i, buffer, count, 0);
                    printf("clientfd: %d, count: %d, buffer: %s\n" , i, count, buffer);

                }
            }
        }
        

    }
