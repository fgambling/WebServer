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


class http_conn {
public:
    
    static int m_epollfd;
    static int m_user_count;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 2048;
    static const int FILENAME_LEN = 200;

    // HTTP请求方法，这里只支持GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE:当前正在分析请求行
        CHECK_STATE_HEADER:当前正在分析头部字段
        CHECK_STATE_CONTENT:当前正在解析请求体
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户数据
        GET_REQUEST         :   表示获得了一个完成的客户请求
        BAD_REQUEST         :   表示客户请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求,获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    

    http_conn() {}
    ~http_conn() {}

    void process();

    void init(int sockfd, const sockaddr_in & addr);
    void close_conn();
    bool read();
    bool write();
    void unmap();
    bool process_write(HTTP_CODE ret);

    HTTP_CODE process_read();
    HTTP_CODE parse_request_line(char * text);
    HTTP_CODE parse_headers(char * text);
    HTTP_CODE parse_content(char * text);
    HTTP_CODE do_request();

    LINE_STATUS parse_line();

    bool add_status_line(int status, const char* title);
    bool add_headers(int content_len);
    bool add_content(const char* content);
    bool add_response(const char* format, ...);
    bool add_content_length(int content_len);
    bool add_content_type();
    bool add_linger();
    bool add_blank_line();
    


private:
    int m_sockfd;
    sockaddr_in m_address; 
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_index;

    int m_check_index;
    int m_start_line;
    char * m_url;
    char * m_version;
    METHOD m_method;
    char * m_host;
    bool m_linger; // connection
    long m_content_length;

    CHECK_STATE m_check_state;

    char m_real_file[FILENAME_LEN];
    struct stat m_file_stat;
    char* m_file_address; 

    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    struct iovec m_iv[2];
    int m_iv_count;

    int bytes_to_send;
    int bytes_have_send; 

    void init();

    char * get_line() {return m_read_buf + m_start_line;}
};




#endif