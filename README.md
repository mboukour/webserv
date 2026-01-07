# Webserv

A high-performance HTTP/1.1 web server written in **C++ 98**, utilizing I/O multiplexing with `epoll` to handle concurrent non-blocking connections.

## 🚀 Features

- **I/O Multiplexing**: Single-threaded event loop using `epoll` for efficient handling of multiple clients.
- **HTTP Methods**: Full support for `GET`, `POST`, and `DELETE`.
- **CGI Support**: Execution of scripts (e.g., PHP, Python) with support for chunked request bodies.
- **Configuration**: Nginx-like configuration file parsing (ports, hostnames, error pages, body size limits, etc.).
- **Static File Serving**: Serving HTML, images, and other static assets.
- **Autoindex**: Automatic directory listing when no index file is present.
- **Redirection**: Support for HTTP 301/302 redirects.
- **Session Management**: Built-in login and session handling via cookies (Bonus).

## 🛠️ Installation

### Prerequisites
- GCC/Clang compiler
- Linux environment (required for `epoll`)

### Build
```
make
```
💻 Usage
Run the server by providing a configuration file:
```
./webserv [path_to_config.conf]
```
Example:
```
./webserv configs/test.conf
```
📂 Project Structure
Server/: Core server logic and connection management.

* **Http:** Request parsing and response generation.

* **Parser:** Configuration file tokenizer and validator.

* **Cgi:** CGI execution and environment setup.

* **EpollEvent:** Wrapper for the Linux epoll interface.

* **Session:** Logic for handling user sessions and authentication.

* **Utils:** Logging and helper functions.

⚙️ Configuration File

The configuration allows defining multiple server blocks:

```Nginx

server {
    listen 8080;
    server_name example.com;
    root ./www;
    index index.html;
    client_max_body_size 10M;

    location /upload {
        allow_methods POST;
        upload_store ./uploads;
    }

    location .php {
        cgi_path /usr/bin/php-cgi;
    }
}
```
📝 License
This project was developed as part of the 42 Network curriculum.
