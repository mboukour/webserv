#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <string>

#include "../../Debug/Debug.hpp"
#include "ServerManager.hpp"
#include "../../Http/HttpRequest/HttpRequest.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"
#include "../../Http/HttpResponse/ResponseState/ResponseState.hpp"
#include "../../Session/Login/Login.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"


ServerManager::ServerManager(std::vector<Server> &servers): servers(servers) {}

bool ServerManager::isAServerFdSocket(int fdSocket) const {
    for (std::vector<Server>::const_iterator it = this->servers.begin();
            it != this->servers.end(); it++) {
        if (it->getFdSocket() == fdSocket)
            return (true);
    }
    return (false);
}


const Server &ServerManager::getServer(int port) {
    for (std::vector<Server>::const_iterator it = this->servers.begin();
        it != this->servers.end(); it++) { 
            if (it->getPort() == port)
                return (*it);
        }
    throw std::logic_error("Server not found"); // throw ServerNotFound(serverFd);
}



std::ostream& operator<<(std::ostream& outputStream, const HttpRequest& request);
void ServerManager::handleClient(int clientFd) {
    static std::map<std::string, std::string> userCreds;
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
        // Wtf is this shit?
        struct sockaddr_in addr;
        socklen_t addrLen = sizeof(addr);
        if (getsockname(clientFd, (struct sockaddr*)&addr, &addrLen) == -1) {
            std::cerr << "Error: getsockname failed. Errno: " << strerror(errno) << std::endl;
            close(clientFd);
            epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            return;
        }
        int port = ntohs(addr.sin_port);
        std::string responseStr;
        try  {
            HttpRequest request(buffer, this->servers, port);
            DEBUG && std::cout << "New request: " << request << std::endl;
            if (request.getPath() == "/session-test")
                Login::respondToLogin(request, userCreds, clientFd);
            else 
                HttpResponse response(request, clientFd, epollFd); // this needs more work-> matching is done via port + server name, we need a server choosing algorithm, send not found if we cant find it!!!
        } catch (const HttpErrorException &exec) {
            DEBUG && std::cerr << "Response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << "\n" << std::endl;
            std::string respStr = exec.getResponseString();
            send(clientFd, respStr.c_str(), respStr.size(), 0);
            std::cout << "clientFd: " << clientFd <<std::endl;
            close(clientFd);
            epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            DEBUG && std::cout << "Connection closed after error: " << buffer << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: recv failed. Errno: " << strerror(errno) << " fd: " << clientFd << std::endl;
        close(clientFd);
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL); // this can fail
    }
}

void ServerManager::acceptConnections(int fdSocket) {
    struct sockaddr_in client_addr;
    std::string errorStr;
    socklen_t client_len = sizeof(client_addr);
    int clientFd = accept(fdSocket, (struct sockaddr *)&client_addr, &client_len);
    if (clientFd == -1)
    {
        errorStr = "Error: accept failed. Errno: " ;
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    
    DEBUG && std::cout << "New connection accepted!" << std::endl;
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1)
    {
        std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
        close(clientFd);
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
            if (events[i].events == EPOLLIN) {
                if (isAServerFdSocket(fd))
                    acceptConnections(fd);
                else
                    handleClient(fd);
            } else if (events[i].events == EPOLLOUT) {
                ResponseState* state = reinterpret_cast<ResponseState *>(events[i].data.ptr);
                state->continueSending();
                delete state;
            }
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

