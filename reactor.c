#include <sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/poll.h>
#include<sys/epoll.h>

#define BUFFER_LENGTH 128


//定义了一个结构体conn_item和一个结构体数组connlist。conn_item结构体包含一个文件描述符fd，一个缓冲区buffer和一个索引idx。connlist数组用于存储所有的客户端连接
struct conn_item{
    int fd;
    char buffer[128];
    int idx;
};

struct conn_item connlist[1024]= {0};


int main()

    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        //定义了一个sockaddr_in结构体变量serveraddr，并将其所有字段初始化为0。
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(struct sockaddr_in));

        //设置了serveraddr的各个字段。其中，sin_family字段设置为AF_INET表示使用IPv4协议，sin_addr.s_addr字段设置为INADDR_ANY表示绑定到所有的网络接口，sin_port字段设置为2048表示监听2048端口。
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(2048);

        //将sockfd套接字绑定到serveraddr指定的地址和端口。
        if (-1 == bind(sockfd,(struct sockaddr*)&serveraddr, sizeof(struct sockaddr)))
        {
            perror("bind");
            return -1;
        }

        listen(sockfd, 10);


        //创建了一个epoll实例,
        int epfd = epoll_create(1); //1这个参数没啥用，只是为了兼容旧的版本

        //创建了一个epoll_event结构体ev，并将其设置为监听sockfd套接字的读事件
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = sockfd;

        //将ev添加到epfd表示的epoll实例中
        epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); //which box, the method, which socket, and events
        

        //定义了一个epoll_event数组events，用于存储epoll_wait函数返回的事件。
        struct epoll_event events[1024] = {0};

        //开始了一个无限循环，服务器将在这个循环中接受客户端的连接请求和数据
        while(1)
        {
            //调用epoll_wait函数等待事件的发生。如果有事件发生，那么epoll_wait函数将返回发生的事件数量，并将这些事件存储在events数组中
            int nready = epoll_wait(epfd, events, 1024,-1 ); //events就像快递员的袋子

            int i = 0;
            for (int i = 0; i < nready; i++)
            {

                //获取了当前事件对应的文件描述符
                int connfd = events[i].data.fd;

                //检查当前事件是否是新的客户端连接请求
                if (sockfd ==connfd)
                {

                //接受了新的客户端连接请求，并返回了一个新的文件描述符clientfd。
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr, &len);

                //将新的客户端连接添加到epfd表示的epoll实例中
                //EPOLLIN表示监听读事件，即监听到数据可读时触发。这是因为在epoll中，只有当有数据可读时，才能调用read函数来从套接字读取数据。
                // EPOLLET表示ET模式。在ET模式下，当有事件发生时，epoll系统调用会立即返回，而不是等待事件队列中有事件时才返回。这种模式可以减少过度的唤醒和系统调用开销，但是需要注意的是，ET模式只能在Linux系统上使用。
                // 因此，EPOLLIN | EPOLLET表示监听
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

                //初始化了connlist[clientfd]
                connlist[clientfd].fd = clientfd;
                memset(connlist[clientfd].buffer,0, BUFFER_LENGTH);
                connlist[clientfd].idx = 0;

                printf("clientfd: %d\n", clientfd);


                }else if (events[i].events & EPOLLIN){
                    
                    //获取了当前连接的缓冲区和索引。
                    char *buffer =connlist[connfd].buffer;
                    int idx = connlist[connfd].idx;

                    //从当前连接接收数据，并返回接收到的字节数。
                    int count = recv(connfd, buffer, BUFFER_LENGTH - idx, 0);
                    if (count ==0){
                        printf("disconnected\n");

                        //从epfd表示的epoll实例中删除了当前连接，并关闭了当前连接。
                        epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);                        
                        close(i);

                        continue;

                    }

                    //更新了当前连接的索引
                    connlist[connfd].idx += count;

                    //将接收到的数据发送回客户端
                    send(connfd, buffer, count, 0);
                    printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buffer);

                }
            }
        }
            //等待用户输入一个字符，然后返回。这是为了防止程序立即退出
            getchar();

        

    }
