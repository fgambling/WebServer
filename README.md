# Lightweight Linux Web Server

A high-performance, event-driven web server implemented in C++ for Linux systems. This server uses the **Proactor** concurrency model to handle thousands of concurrent connections efficiently.

## 🚀 Features

### Core Architecture

- **Event-Driven I/O**: Uses Linux `epoll` for high-performance event notification
- **Multi-Threading**: Thread pool for concurrent request processing
- **Non-Blocking I/O**: Efficient handling of multiple concurrent connections
- **HTTP/1.1 Support**: Full HTTP protocol implementation with GET method support

### Performance Features

- **High Concurrency**: Supports thousands of simultaneous connections
- **Memory-Mapped Files**: Efficient static file serving
- **Connection Pooling**: Reuses connection objects to minimize overhead
- **Finite State Machine**: Robust HTTP request parsing

### Technical Features

- **Socket Programming**: Inter-host communication capabilities
- **Signal Handling**: Graceful shutdown and error handling
- **Resource Management**: RAII design for automatic cleanup
- **Error Handling**: Comprehensive HTTP status code support

## 📁 Project Structure

```
WebServer/
├── main.cpp              # Main server entry point and event loop
├── http_conn.h          # HTTP connection class header
├── http_conn.cpp        # HTTP connection implementation
├── threadpool.h         # Thread pool template header
├── threadpool.cpp       # Thread pool implementation
├── locker.h             # Synchronization primitives (mutex, semaphore)
├── resources/           # Static files served by the server
│   ├── index.html      # Default web page
│   └── images/         # Image files
└── README.md           # This file
```

## 🏗️ Architecture Overview

### Event-Driven Architecture

The server uses Linux's `epoll` for efficient event notification:

- **Main Event Loop**: Monitors file descriptors for read/write events
- **Non-Blocking I/O**: Prevents blocking on individual connections
- **Event Types**: Handles read events, write events, and connection errors

### Thread Pool Design

- **Producer-Consumer Pattern**: Tasks added to queue, processed by worker threads
- **Semaphore Synchronization**: Efficient task distribution
- **Configurable Thread Count**: Default 8 threads, customizable
- **Automatic Cleanup**: RAII design prevents resource leaks

### HTTP Processing Pipeline

1. **Connection Acceptance**: New client connections are accepted and initialized
2. **Request Reading**: Non-blocking read of HTTP request data
3. **Request Parsing**: Finite state machine parses HTTP headers and body
4. **Response Generation**: HTTP response is generated based on request
5. **Response Writing**: Non-blocking write of response data

## 🔧 Building the Project

### Prerequisites

- Linux operating system
- GCC compiler with C++ support
- POSIX threads library
- Make utility (optional)

### Compilation

```bash
# Compile all source files
g++ -o webserver main.cpp http_conn.cpp threadpool.cpp -lpthread

# Or compile with optimization flags
g++ -O2 -o webserver main.cpp http_conn.cpp threadpool.cpp -lpthread
```

### Build Options

- `-O2`: Enable optimization for better performance
- `-lpthread`: Link against POSIX threads library
- `-Wall`: Enable all warnings (recommended for development)

## 🚀 Running the Server

### Basic Usage

```bash
# Start server on port 8080
./webserver 8080

# Start server on port 80 (requires root privileges)
sudo ./webserver 80
```

### Configuration

- **Port Number**: Specify as command line argument
- **Document Root**: Currently hardcoded to `/home/ubuntu/WebServer/resources`
- **Thread Pool Size**: Default 8 threads, configurable in `threadpool.h`
- **Max Connections**: Limited by `Max_FD` constant (65535)

## 📊 Performance Characteristics

### Concurrency Model

- **Event-Driven**: Single-threaded event loop with epoll
- **Thread Pool**: Worker threads handle request processing
- **Non-Blocking**: All I/O operations are non-blocking
- **Connection Reuse**: Connection objects are reused to minimize overhead

### Scalability Features

- **Epoll Efficiency**: O(1) event notification
- **Memory Mapping**: Efficient file serving without copying
- **Buffer Management**: Configurable read/write buffer sizes
- **Connection Limits**: Configurable maximum connection count

## 🔍 HTTP Protocol Support

### Supported Methods

- **GET**: Full implementation with file serving
- **HEAD**: Basic support (returns headers only)
- **POST**: Framework ready (not fully implemented)
- **Other Methods**: Framework exists for future implementation

### HTTP Features

- **HTTP/1.1**: Full protocol compliance
- **Keep-Alive**: Connection persistence support
- **Status Codes**: 200, 400, 403, 404, 500
- **Headers**: Content-Type, Content-Length, Connection

### File Serving

- **Static Files**: HTML, CSS, JavaScript, images
- **Memory Mapping**: Efficient file delivery
- **Error Handling**: Proper 404 and 403 responses
- **Security**: Basic path validation

## 🛠️ Technical Implementation

### Key Classes

#### `http_conn`

- **Purpose**: Handles individual HTTP client connections
- **Features**: Request parsing, response generation, connection management
- **State Machine**: Finite state machine for HTTP parsing
- **Memory Management**: Automatic cleanup of resources

#### `threadpool<T>`

- **Purpose**: Template-based thread pool for concurrent processing
- **Features**: Producer-consumer pattern, semaphore synchronization
- **Configurability**: Adjustable thread count and queue size
- **Thread Safety**: Mutex-protected work queue

#### `locker`, `cond`, `signal`

- **Purpose**: RAII wrappers for POSIX synchronization primitives
- **Features**: Automatic resource management, exception safety
- **Usage**: Thread pool synchronization, connection management

### Synchronization Primitives

- **Mutex (`locker`)**: Protects shared resources
- **Condition Variable (`cond`)**: Thread signaling and waiting
- **Semaphore (`signal`)**: Task counting and synchronization

## 🔧 Configuration and Customization

### Server Parameters

```cpp
// In main.cpp
#define Max_FD 65535        // Maximum file descriptors
#define Max_event 10000     // Maximum epoll events

// In threadpool.h
threadpool(int num = 8, int max_num = 10000)  // Thread count and queue size
```

### Buffer Sizes

```cpp
// In http_conn.h
static const int READ_BUFFER_SIZE = 2048;   // HTTP request buffer
static const int WRITE_BUFFER_SIZE = 2048;  // HTTP response buffer
static const int FILENAME_LEN = 200;        // Maximum filename length
```

### Document Root

```cpp
// In http_conn.cpp
const char* doc_root = "/home/ubuntu/WebServer/resources";
```

## 🐛 Troubleshooting

### Common Issues

#### Port Already in Use

```bash
# Check what's using the port
sudo netstat -tulpn | grep :8080

# Kill the process or use a different port
sudo kill -9 <PID>
```

#### Permission Denied

```bash
# For ports below 1024, run with sudo
sudo ./webserver 80

# Or use a higher port number
./webserver 8080
```

#### Compilation Errors

```bash
# Ensure pthread library is linked
g++ -o webserver *.cpp -lpthread

# Check for missing headers
g++ -Wall -o webserver *.cpp -lpthread
```

### Debugging

- **Verbose Output**: Check console for connection and parsing messages
- **Error Logs**: Server prints error messages to stdout
- **Connection Monitoring**: Use `netstat` or `ss` to monitor connections

## 📈 Performance Tuning

### Thread Pool Optimization

- **CPU-Bound Workloads**: Increase thread count to CPU core count
- **I/O-Bound Workloads**: Increase thread count beyond CPU cores
- **Memory Usage**: Monitor memory usage with high thread counts

### Buffer Optimization

- **Large Files**: Increase `WRITE_BUFFER_SIZE` for better throughput
- **High Concurrency**: Adjust `READ_BUFFER_SIZE` based on request size
- **Memory Usage**: Balance buffer size with memory constraints

### Connection Limits

- **File Descriptors**: Monitor system limits with `ulimit -n`
- **Epoll Events**: Adjust `Max_event` based on expected concurrency
- **Connection Pool**: Monitor `Max_FD` usage under load

## 🔮 Future Enhancements

### Planned Features

- **HTTP/2 Support**: Modern protocol implementation
- **SSL/TLS**: Secure connections with HTTPS
- **Dynamic Content**: CGI or FastCGI support
- **Load Balancing**: Multiple server instances
- **Configuration File**: Flexible server configuration
- **Logging System**: Comprehensive request logging

### Performance Improvements

- **Zero-Copy I/O**: Use `sendfile()` for file transfers
- **HTTP Pipelining**: Support for multiple requests per connection
- **Compression**: Gzip/deflate support for responses
- **Caching**: Response caching for static content

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 📚 References

- [Linux epoll Documentation](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [HTTP/1.1 Specification](https://tools.ietf.org/html/rfc2616)
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [Linux Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)
