#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include <signal.h>
#include "http_conn.h"

#define Max_FD 65535
#define Max_event 10000

void addsig(int sig, void(handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

extern void addfd(int epollfd, int fd, bool one_shot);

extern void removefd(int epollfd, int fd);

extern void modfd(int epollfd, int fd, int ev);

int main(int argc, char* argv[]) {

    if (argc <= 1)
    {
        printf("按如下格式运行: %s port_number\n", basename(argv[0]));
        exit(-1);
    }

    int port = atoi(argv[1]);

    addsig(SIGPIPE, SIG_IGN);

    threadpool<http_conn> * pool = NULL;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch(...)
    {
        exit(-1);
    }

    http_conn * users = new http_conn[Max_FD];

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    int reuse = 1;
    

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);  
    bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    
    listen(listenfd, 5);

    epoll_event events[Max_event];
    int epollfd = epoll_create(2);

    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while (true)
    {
        int num = epoll_wait(epollfd, events, Max_event, -1);

        // if ( ( num < 0 ) && ( errno != EINTR ) ) {
        //     printf( "epoll failure\n" );
        //     break;
        // }

        for (int i =0; i < num; i++)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);

                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);

                if (http_conn::m_user_count >= Max_FD)
                {
                    close(connfd);
                    continue;
                }

                users[connfd].init(connfd, client_address);
            }

            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                users[sockfd].close_conn();
            }

            else if (events[i].events & EPOLLIN)
            {
                if (users[sockfd].read())
                {
                    pool->append(users + sockfd);
                }
                else
                {
                    users[sockfd].close_conn();
                }
            }

            else if (events[i].events & EPOLLOUT)
            {
                if (!users[sockfd].write())
                {
                    users[sockfd].close_conn();
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    return 0;
}