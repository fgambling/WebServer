/**
 * @file http_conn.h
 * @brief HTTP connection class header file
 * @details Defines the http_conn class which handles individual HTTP client connections,
 *          including request parsing, response generation, and connection management.
 * 
 * Key Features:
 * - Finite state machine for HTTP request parsing
 * - Support for HTTP GET requests
 * - Memory-mapped file serving
 * - Non-blocking I/O with epoll integration
 * - Connection pooling and resource management
 */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

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
#include <sys/mman.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/uio.h>

/**
 * @class http_conn
 * @brief HTTP connection handler class
 * @details Manages individual HTTP client connections, including request parsing,
 *          response generation, and connection lifecycle management.
 */
class http_conn {
public:
    
    // Static members for global state management
    static int m_epollfd;        ///< Global epoll file descriptor
    static int m_user_count;     ///< Current number of active connections
    
    // Buffer and file size constants
    static const int READ_BUFFER_SIZE = 2048;   ///< Size of read buffer
    static const int WRITE_BUFFER_SIZE = 2048;  ///< Size of write buffer
    static const int FILENAME_LEN = 200;        ///< Maximum filename length

    /**
     * @enum METHOD
     * @brief HTTP request methods enumeration
     * @details Currently only GET is fully implemented
     */
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /**
     * @enum CHECK_STATE
     * @brief Main state machine states for HTTP request parsing
     * @details Defines the three main parsing states for processing HTTP requests
     */
    enum CHECK_STATE { 
        CHECK_STATE_REQUESTLINE = 0,  ///< Currently parsing the request line
        CHECK_STATE_HEADER,           ///< Currently parsing HTTP headers
        CHECK_STATE_CONTENT           ///< Currently parsing request body
    };
    
    /**
     * @enum LINE_STATUS
     * @brief Line parsing status for the sub-state machine
     * @details Indicates the result of parsing a single line from the HTTP request
     */
    enum LINE_STATUS { 
        LINE_OK = 0,    ///< Successfully parsed a complete line
        LINE_BAD,       ///< Line contains syntax errors
        LINE_OPEN       ///< Line is incomplete, need more data
    };

    /**
     * @enum HTTP_CODE
     * @brief HTTP request processing result codes
     * @details Defines all possible outcomes of HTTP request processing
     */
    enum HTTP_CODE { 
        NO_REQUEST,           ///< Request is incomplete, need more data
        GET_REQUEST,          ///< Successfully parsed a complete GET request
        BAD_REQUEST,          ///< Request has syntax errors
        NO_RESOURCE,          ///< Requested resource not found
        FORBIDDEN_REQUEST,    ///< Client lacks permission for requested resource
        FILE_REQUEST,         ///< File request successful, ready to serve
        INTERNAL_ERROR,       ///< Server internal error occurred
        CLOSED_CONNECTION     ///< Client has closed the connection
    };
    
    // Constructor and destructor
    http_conn() {}
    ~http_conn() {}

    // Public interface methods
    void process();           ///< Main processing method called by thread pool

    // Connection management
    void init(int sockfd, const sockaddr_in & addr);  ///< Initialize connection
    void close_conn();        ///< Close and cleanup connection
    bool read();              ///< Read data from client
    bool write();             ///< Write response to client
    void unmap();             ///< Unmap memory-mapped files

    // HTTP processing methods
    bool process_write(HTTP_CODE ret);  ///< Process and send HTTP response

    // Request parsing methods
    HTTP_CODE process_read();           ///< Main request parsing method
    HTTP_CODE parse_request_line(char * text);  ///< Parse HTTP request line
    HTTP_CODE parse_headers(char * text);       ///< Parse HTTP headers
    HTTP_CODE parse_content(char * text);       ///< Parse request body
    HTTP_CODE do_request();             ///< Process the parsed request

    // Line parsing utility
    LINE_STATUS parse_line();           ///< Parse a single line from request

    // Response generation methods
    bool add_status_line(int status, const char* title);  ///< Add HTTP status line
    bool add_headers(int content_len);                    ///< Add HTTP headers
    bool add_content(const char* content);                ///< Add response content
    bool add_response(const char* format, ...);           ///< Add formatted response
    bool add_content_length(int content_len);             ///< Add Content-Length header
    bool add_content_type();                              ///< Add Content-Type header
    bool add_linger();                                    ///< Add Connection header
    bool add_blank_line();                                ///< Add blank line after headers

private:
    // Connection information
    int m_sockfd;           ///< Client socket file descriptor
    sockaddr_in m_address;  ///< Client address information
    
    // Read buffer and parsing state
    char m_read_buf[READ_BUFFER_SIZE];  ///< Buffer for incoming HTTP request
    int m_read_index;                   ///< Current read buffer position
    int m_check_index;                  ///< Current parsing position
    int m_start_line;                   ///< Start of current line being parsed
    
    // HTTP request parsing results
    char * m_url;           ///< Parsed URL from request
    char * m_version;       ///< HTTP version from request
    METHOD m_method;        ///< HTTP method (GET, POST, etc.)
    char * m_host;          ///< Host header value
    bool m_linger;          ///< Connection keep-alive flag
    long m_content_length;  ///< Content-Length header value

    // State machine state
    CHECK_STATE m_check_state;  ///< Current parsing state

    // File serving information
    char m_real_file[FILENAME_LEN];  ///< Full path to requested file
    struct stat m_file_stat;         ///< File statistics
    char* m_file_address;            ///< Memory-mapped file address

    // Write buffer and response state
    char m_write_buf[WRITE_BUFFER_SIZE];  ///< Buffer for HTTP response
    int m_write_idx;                       ///< Current write buffer position
    struct iovec m_iv[2];                  ///< I/O vectors for writev
    int m_iv_count;                        ///< Number of I/O vectors

    // Response tracking
    int bytes_to_send;      ///< Total bytes to send in response
    int bytes_have_send;    ///< Bytes already sent

    // Private initialization method
    void init();            ///< Initialize connection state

    // Utility method
    char * get_line() {return m_read_buf + m_start_line;}  ///< Get current line being parsed
};

#endif