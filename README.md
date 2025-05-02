Project Description: Developed a web server in C++ under Linux, simulating the Proactor high concurrency model. The server supports a relative number of client concurrent accesses and timely responses, supporting parsing of HTTP headers and responding to HTTP GET requests.
*Utilized Sockets for inter-host communication, enabling the server to handle multiple client connections simultaneously.
*Implemented a multi-threading mechanism to provide parallel services, managing threads efficiently using a thread pool.
*Increased server responsiveness by simulating high concurrency through non-blocking I/O and event-driven programming.
*Designed a finite state machine (FSM) to process and parse incoming HTTP GET requests, extracting URL and HTTP header parameters accurately.
*Improved server scalability and performance by utilizing multi-threading and reducing resource contention.
