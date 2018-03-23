#include<stdio.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<ctype.h>
#include<iostream>

using namespace std;

#define MAXLEN 1024
#define SERV_PORT 8000
#define MAX_OPEN_FD 1024

int main(int argc, char *argv[])
{
    int listenfd, connfd, efd, ret;
    char buf[MAXLEN];
    struct sockaddr_in cliaddr, servaddr;
    socklen_t chilen = sizeof(cliaddr);
    struct epoll_event tep, ep[MAX_OPEN_FD];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 20);

    efd = epoll_create(MAX_OPEN_FD);
    /* tep.events = EPOLLIN; */
    tep.events = EPOLLIN | EPOLLET;
    tep.data.fd = listenfd;

    ret = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);

    for(;;)
    {
        size_t nready = epoll_wait(efd, ep, MAX_OPEN_FD, -1);
        for (int i = 0; i< nready; ++i)
        {
            cout << "epoll_wait succ, num:" << nready << endl;
            if (ep[i].data.fd == listenfd)
            {
                connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &chilen);
                cout << "accept new connect, ip(port):" << inet_ntoa(cliaddr.sin_addr) << ":" << cliaddr.sin_port << endl;
                tep.events = EPOLLIN | EPOLLET;
                tep.data.fd = connfd;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
            }
            else if (ep[i].events & EPOLLIN)
            {
                cout << "EPOLLIN event" << endl;
                connfd = ep[i].data.fd;
                int bytes = read(connfd, buf, MAXLEN);
                if (0 == bytes)
                {
                    cout << "close connect" << endl;
                    ret = epoll_ctl(efd, EPOLL_CTL_DEL, connfd, NULL);
                    close(connfd);
                }

                for (int j = 0; j < bytes; ++j)
                {
                    buf[j] = toupper(buf[j]);
                }
                write(connfd, buf, bytes);
            } else {
                cout << "unknown event" << endl;
            }
        }
    }
}
