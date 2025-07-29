/**
 * @file main.cpp
 * @brief Main entry point for the lightweight Linux web server
 * @details This file implements the main server loop using epoll for high-performance
 *          event-driven I/O and a thread pool for concurrent request processing.
 * 
 * Architecture Overview:
 * - Uses epoll for efficient event notification
 * - Thread pool for concurrent request processing
 * - Non-blocking I/O for high concurrency
 * - HTTP connection management with finite state machine
 */

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

// Maximum file descriptor limit for the server
#define Max_FD 65535
// Maximum number of events that epoll can handle at once
#define Max_event 10000

/**
 * @brief Signal handler setup function
 * @param sig Signal number to handle
 * @param handler Function pointer to the signal handler
 * @details Sets up signal handling for graceful server shutdown
 */
void addsig(int sig, void(handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

// External function declarations for epoll management
extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd(int epollfd, int fd);
extern void modfd(int epollfd, int fd, int ev);

/**
 * @brief Main function - Server entry point
 * @param argc Command line argument count
 * @param argv Command line arguments array
 * @return 0 on successful execution, -1 on error
 * 
 * Server initialization and main event loop:
 * 1. Parse command line arguments (port number)
 * 2. Set up signal handling
 * 3. Initialize thread pool for concurrent request processing
 * 4. Create and configure listening socket
 * 5. Set up epoll for event-driven I/O
 * 6. Enter main event loop for handling client connections
 */
int main(int argc, char* argv[]) {

    // Validate command line arguments
    if (argc <= 1)
    {
        printf("Usage: %s port_number\n", basename(argv[0]));
        exit(-1);
    }

    int port = atoi(argv[1]);

    // Ignore SIGPIPE signal to prevent server crash on client disconnect
    addsig(SIGPIPE, SIG_IGN);

    // Initialize thread pool for concurrent request processing
    threadpool<http_conn> * pool = NULL;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch(...)
    {
        exit(-1);
    }

    // Allocate HTTP connection objects for all possible file descriptors
    http_conn * users = new http_conn[Max_FD];

    // Create listening socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);

    // Enable address reuse to avoid "Address already in use" error
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 
    
    // Configure server address structure
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    address.sin_port = htons(port);        // Set port number
    
    // Bind socket to address and port
    bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    
    // Start listening for incoming connections
    listen(listenfd, 5);

    // Create epoll instance for event-driven I/O
    epoll_event events[Max_event];
    int epollfd = epoll_create(2);

    // Add listening socket to epoll for monitoring new connections
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    // Main event loop - handles all client connections and requests
    while (true)
    {
        // Wait for events from epoll
        int num = epoll_wait(epollfd, events, Max_event, -1);

        // Process all events that occurred
        for (int i =0; i < num; i++)
        {
            int sockfd = events[i].data.fd;
            
            // Handle new client connection
            if (sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);

                // Accept new connection
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);

                // Check if we've reached maximum connection limit
                if (http_conn::m_user_count >= Max_FD)
                {
                    close(connfd);
                    continue;
                }

                // Initialize HTTP connection object for new client
                users[connfd].init(connfd, client_address);
            }

            // Handle connection errors or client disconnection
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                users[sockfd].close_conn();
            }

            // Handle incoming data (client request)
            else if (events[i].events & EPOLLIN)
            {
                if (users[sockfd].read())
                {
                    // Add request to thread pool for processing
                    pool->append(users + sockfd);
                }
                else
                {
                    // Close connection if read failed
                    users[sockfd].close_conn();
                }
            }

            // Handle outgoing data (server response)
            else if (events[i].events & EPOLLOUT)
            {
                if (!users[sockfd].write())
                {
                    // Close connection if write failed
                    users[sockfd].close_conn();
                }
            }
        }
    }

    // Cleanup resources
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    return 0;
}