#include "ServerManager.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <string>
#include <iostream>


ServerManager::ServerManager(std::vector<Server> &servers): servers(servers) {}

bool ServerManager::isAServerFdSocket(int fdSocket) const {
    for (std::vector<Server>::const_iterator it = this->servers.begin();
            it != this->servers.end(); it++) {
        if (it->getFdSocket() == fdSocket)
            return (true);
    }
    return (false);
}

void ServerManager::handleClient(int clientFd) {
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
        // Response goes here
        buffer[bytesReceived] = '\0';
        DEBUG && std::cout << "Client: " << buffer << std::endl;
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(clientFd, response.c_str(), response.size(), 0);
    }
    else
    {
        std::cerr << "Error: recv failed. Errno: " << strerror(errno) << std::endl;
        close(clientFd);
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL); // this can fail
    }
}

void ServerManager::acceptConnections(int fdSocket) {
    struct sockaddr_in client_addr;
    std::string errorStr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(fdSocket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket == -1)
    {
        errorStr = "Error: accept failed. Errno: " ;
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    fcntl(client_socket, F_SETFL, O_NONBLOCK);
    
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

void ServerManager::handleConnections(void) {
     struct epoll_event events[MAX_EVENTS];
    std::string errorStr;

    while (true)
    {
        // -1 means wait indefinitely

        int event_count = epoll_wait(this->epollFd, events, MAX_EVENTS, -1);
        if (event_count == -1)
        {
            errorStr = "epoll_wait() failed. Errno: ";
            errorStr += strerror(errno);
            throw std::runtime_error(errorStr);
        }
        for (int i = 0; i < event_count; i++)
        {
            int fd = events[i].data.fd;
            if (isAServerFdSocket(fd))
                acceptConnections(fd);
            else
                handleClient(fd);
        }

        }
}


void ServerManager::startServerManager(void) {
    this->epollFd = epoll_create(1);
    if (this->epollFd == -1)
    {
        std::string errorStr = "epoll_create() failed";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    for (std::vector<Server>::iterator it = this->servers.begin();
        it != this->servers.end(); it++)
    {
        struct epoll_event ev;

        it->startServer();
        int fdSocket = it->getFdSocket();
        ev.events = EPOLLIN;
        ev.data.fd = fdSocket;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fdSocket, &ev) == -1)
        {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(fdSocket);
        }
    }
    handleConnections();
}

