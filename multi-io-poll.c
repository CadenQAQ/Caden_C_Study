#include <sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/poll.h>
#include<sys/epoll.h>

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

#if 0
        struct pollfd fds[1024] = {0};

        fds[sockfd].fd = sockfd;
        fds[sockfd].events = POLLIN;

        int maxfd = sockfd;

        while(1){

            int nready = poll(fds,maxfd+1,-1);

            if(fds[sockfd].revents & POLLIN)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr, &len);
                printf("sockfd: %d\n", clientfd);

                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;

                maxfd = clientfd;
            }

            int i = 0;
            for (i = sockfd+1 ; i<=maxfd; i++)
            {
                if (fds[i].revents & POLLIN)
                {
                    char buffer[128] = {0};
                    int count = recv(i, buffer, 128, 0);

                    if(count ==0)
                    {
                        printf("disconnected\n");
                        fds[i].fd = -1;
                        fds[i].events = 0;

                        close(i);

                        break;
                    }

                }
            }

        }

    #else
        int epfd = epoll_create(1); //1这个参数没啥用，只是为了兼容旧的版本
        
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = sockfd;

        epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); //which box, the method, which socket, and events
        
        struct epoll_event events[1024] = {0};
        while(1)
        {
            int nready = epoll_wait(epfd, events, 1024,-1 ); //events就像快递员的袋子

            int i = 0;
            for (int i = 0; i < nready; i++)
            {
                int connfd = events[i].data.fd;
                if (sockfd ==connfd)
                {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr, &len);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
                printf("clientfd: %d\n", clientfd);


                }else if (events[i].events & EPOLLIN){

                    char buffer[128] ={0};
                    int count = recv(connfd, buffer, 128, 0);
                    if (count ==0){
                        printf("disconnected\n");

                        epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);                        
                        close(i);

                        continue;

                    }

                    send(connfd, buffer, count, 0);
                    printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buffer);

                }
            }
        }


    #endif
            getchar();

        

    }
