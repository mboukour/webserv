#include "Server.hpp"
#include <iostream> // to remove later
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <csignal>
#include "../Debug/Debug.hpp"

void Server::setPort(int port) {this->port = port;}

void Server::setServerName(const std::string &serverName) {this->serverName = serverName;}

int Server::getPort(void) const {return this->port;}

std::string Server::getServerName(void) const {return this->serverName;}


void Server::startServer(void)  {

    std::string errorStr;

    this->fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fdSocket == -1)
        throw std::runtime_error("Socket creation failed");
    int opt = 1;
    if (setsockopt(this->fdSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        close(this->fdSocket);
        errorStr = "setsockopt() failed. Errno: ";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(this->fdSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        close(this->fdSocket);
        throw std::runtime_error("bind(): socket binding failed");
    }
    if (listen(this->fdSocket, 128) == -1)
    {
        close(this->fdSocket);
        throw std::runtime_error("listen(): failed");
    }
    DEBUG && std::cout << "Socket bound successfully to port " << this->port << std::endl;
    this->epollFd = epoll_create(1);
    if (this->epollFd == -1)
    {
        errorStr = "epoll_create() failed";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = this->fdSocket;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->fdSocket, &ev) == -1)
    {
        std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
        close(this->fdSocket);
    }
    handleConnections();
}

void Server::handleConnections(void) const {
    struct epoll_event events[MAX_EVENTS];
    std::string errorStr;

    while (true)
    {
        //the MAX_EVENTS parameter is the maximum number of events that can be returned by epoll_wait
        // in one iteration of the loop
        int event_count = epoll_wait(this->epollFd, events, MAX_EVENTS, -1);
        if (event_count == -1)
        {
            errorStr = "epoll_wait() failed. Errno: ";
            errorStr += strerror(errno);
            throw std::runtime_error(errorStr);
        }
        for (int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == this->fdSocket)
                acceptConnections();
            else
                handleClient(events[i].data.fd);
        }
    }
}

void Server::handleClient(int clientFd) const {
    char buffer[1024] = {0};
    ssize_t bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0)
    {
        DEBUG && std::cout << "Client disconnected" << std::endl;
        close(clientFd);
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    }
    else if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        DEBUG && std::cout << "Client: " << buffer << std::endl;
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(clientFd, response.c_str(), response.size(), 0);
    }
    else
    {
        std::cerr << "Error: recv failed. Errno: " << strerror(errno) << std::endl;
        close(clientFd);
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    }
}

void Server::acceptConnections(void) const {
    struct sockaddr_in client_addr;
    std::string errorStr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(this->fdSocket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket == -1)
    {
        errorStr = "Error: accept failed. Errno: " ;
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }

    DEBUG && std::cout << "New connection accepted!" << std::endl;
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_socket;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, client_socket, &ev) == -1)
    {
        std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
        close(client_socket);
    }
}