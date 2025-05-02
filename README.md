## Project Description

Developed a web server in C++ under Linux, simulating the **Proactor** high concurrency model. The server supports a large number of concurrent client accesses and timely responses. It includes:

- Parsing of HTTP headers and responding to HTTP `GET` requests.
- **Socket programming** for inter-host communication, enabling the server to handle multiple client connections simultaneously.
- **Multi-threading** with an efficient thread pool to provide parallel processing capabilities.
- **Non-blocking I/O** and **event-driven programming** to simulate high concurrency and improve responsiveness.
- A **finite state machine (FSM)** for processing and parsing incoming HTTP requests, accurately extracting URLs and header parameters.
- Enhanced scalability and performance by reducing resource contention and optimizing concurrency mechanisms.

