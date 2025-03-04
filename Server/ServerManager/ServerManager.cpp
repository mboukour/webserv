#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <typeinfo>
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
#include "../../ConnectionState/ConnectionState.hpp"

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


void ServerManager::sendResponse(HttpRequest &request, int clientFd) {
    static std::map<std::string, std::string> userCreds;

    try  {
        std::cout << "METHOD: " << request.getMethod() << '\n';
        // DEBUG && std::cout << "New request: " << request << std::endl;
        if (request.getPath() == "/session-test")
            Login::respondToLogin(request, userCreds, clientFd);
        else {
            try{
                const Location &loc = dynamic_cast<const Location &>(*request.getRequestBlock());
                if (loc.getIsReturnLocation()) {
                    HttpResponse response(request.getVersion(), loc.getReturnCode(), "Moved Permanently", "");
                    response.setHeader("Location", loc.getReturnPath());
                    std::string respStr = response.toString();
                    std::cout << respStr << '\n';
                    send(clientFd, respStr.c_str(), respStr.size(), 0);
                    return ;
                }
            } catch (std::bad_cast &) {}
            HttpResponse response(request, clientFd, epollFd); // this needs more work-> matching is done via port + server name, we need a server choosing algorithm, send not found if we cant find it!!!
        }
    } catch (const HttpErrorException &exec) {
        DEBUG && std::cerr << "Response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << "\n" << std::endl;
        std::string respStr = exec.getResponseString();
        send(clientFd, respStr.c_str(), respStr.size(), 0);
        std::cout << "clientFd: " << clientFd <<std::endl;
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, clientFd, NULL);
        close(clientFd);
        DEBUG && std::cout << "Connection closed after error: " << strerror(errno) << std::endl;
    }
}

std::ostream& operator<<(std::ostream& outputStream, const HttpRequest& request);

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
    // ev.data.fd = clientFd;
    ev.data.ptr = new ConnectionState(clientFd, epollFd);
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
            ConnectionState *state = static_cast<ConnectionState *>(events[i].data.ptr);
            int eventFd = state->getEventFd();
            if (events[i].events & EPOLLIN) {
                if (isAServerFdSocket(eventFd))
                    acceptConnections(eventFd);
                else
                {
                    state->handleReadable(this->servers);
                    if (state->getIsRequestReady())
                    {
                        HttpRequest * request = state->getHttpRequest();
                        sendResponse(*request, eventFd);
                        if (state->getIsRequestDone())
                            delete request;
                    }
                }
            }

            if (events[i].events & EPOLLOUT) {
                state->handleWritable();
            }
            if (state->getIsDone())
                delete state;
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
        ev.data.ptr = new ConnectionState(fdSocket, epollFd);
        if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fdSocket, &ev) == -1)
        {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(fdSocket);
        }
    }
    handleConnections();
}

