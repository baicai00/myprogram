#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

using namespace std;

#define LISTENQ 20
#define MAXLINE 5

void setnonblocking(int sock)
{
    int opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }

    opts |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int portnumber = 6666;

    // 声明epoll_event结构体的变量,ev用于注册事件，数组用于回传要处理的事件
    struct epoll_event ev, events[20];
    // 生成用于处理accept的epoll专用文件描述符
    int epfd = epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    socklen_t clilen;
    char line[MAXLINE] = {0};
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 把socket设置为非阻塞方式
    //setnonblocking(listenfd);

    // 设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;
    // 设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;

    // 注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    char* local_addr = "127.0.0.1";
    inet_aton(local_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(portnumber);
    bind(listenfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);

    for (;;)
    {
        // 等待epoll事件的发生
        int nfds = epoll_wait(epfd, events, 20, -1);
        cout << "wake up: " << nfds << endl;

        // 处理所发生的所有事件
        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == listenfd) // 如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接
            {
                int connfd = accept(listenfd, (sockaddr*)&clientaddr, &clilen);
                if (connfd < 0)
                {
                    perror("connfd < 0");
                    //cout << errno << endl;
                    exit(-1);
                }

                char* str = inet_ntoa(clientaddr.sin_addr);
                cout << "accept a connection from " << str << endl;

                // 设置用于读操作的文件描述符
                ev.data.fd = connfd;
                // 设置用于注册的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                // 注册ev
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (events[i].events & EPOLLIN) // 如果是已连接用户，并且收到数据，那么进行读入
            {
                cout << "EPOLLIN" << endl;
                if (events[i].data.fd < 0)
                {
                    continue;
                }
                int n = 0;
                if ((n = read(events[i].data.fd, line, MAXLINE)) < 0)
                {
                    if (errno == ECONNRESET)
                    {
                        close(events[i].data.fd);
                        events[i].data.fd = -1;
                    }
                    else
                    {
                        cout << "readline error" << endl;
                    }
                }
                else if ( n == 0 )
                {
                    close(events[i].data.fd);
                    events[i].data.fd = -1;
                    cout << "client close socket" << endl;
                }
                line[n] = '\n';
                cout << "read: " << line << endl;
            }
            else if (events[i].events & EPOLLOUT)
            {
                // 暂不处理
            }
        }
    }

    return 0;
}