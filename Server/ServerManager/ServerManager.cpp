#include <cstddef>
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
#include <vector>

#include "../../Debug/Debug.hpp"
#include "ServerManager.hpp"
#include "../../Http/HttpRequest/HttpRequest.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"
#include "../../Session/Login/Login.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../ConnectionState/ConnectionState.hpp"
#include "../../Utils/Logger/Logger.hpp"
int ServerManager::epollFd = -1;
std::vector<Server> ServerManager::servers;
std::map<int, ConnectionState*> ServerManager::clientStates;

ServerManager::ServerManager() {}

void ServerManager::initialize(std::vector<Server> &serversList) {
    epollFd = -1;
    servers = serversList;
}

void ServerManager::sendString(const std::string &str, int clientFd) {
    ConnectionState *state = getConnectionState(clientFd);
    state->activateWriteState(str);
}

void ServerManager::sendFile(const std::string &filePath, int clientFd) {
    ConnectionState *state = getConnectionState(clientFd);
    state->activateWriteState(filePath, 0);
}

bool ServerManager::isAServerFdSocket(int fdSocket) {
    for (std::vector<Server>::const_iterator it = servers.begin();
            it != servers.end(); it++) {
        if (it->getFdSocket() == fdSocket)
            return (true);
    }
    return (false);
}

const Server &ServerManager::getServer(int port) {
    for (std::vector<Server>::const_iterator it = servers.begin();
        it != servers.end(); it++) { 
            if (it->getPort() == port)
                return (*it);
        }
    throw std::logic_error("Server not found"); // throw ServerNotFound(serverFd);
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
    
    DEBUG && std::cout << "New connection accepted: " << clientFd << std::endl;
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ConnectionState *state = new ConnectionState(clientFd, epollFd);
    clientStates[clientFd] = state;
    ev.data.ptr = state;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1)
    {
        std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
        close(clientFd);
    }
}

ConnectionState *ServerManager::getConnectionState(int clientFd) {
    return clientStates.at(clientFd);
}

void ServerManager::handleConnections(void) {
    struct epoll_event events[MAX_EVENTS];
    std::string errorStr;

    const int EPOLL_TIMEOUT_MS = 1000;
    while (true)
    {
        int event_count = epoll_wait(epollFd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
        if (event_count == -1)
        {
            errorStr = "epoll_wait() failed. Errno: ";
            errorStr += strerror(errno);
            std::cerr << errorStr << '\n';
        }
        for (int i = 0; i < event_count; i++)
        {
            ConnectionState *state = static_cast<ConnectionState *>(events[i].data.ptr);
            int eventFd = state->getEventFd();
            if (events[i].events & EPOLLIN) {
                if (isAServerFdSocket(eventFd))
                    acceptConnections(eventFd);
                else
                    state->handleReadable(servers);
            }

            if (events[i].events & EPOLLOUT) {
                state->handleWritable();
            }
            if ((state->getIsDone() && state->isSendingDone()) || !state->getIsKeepAlive()) {
                Logger::getLogStream() << "Deleteting state: " << eventFd << std::endl;
                delete state;
                std::map<int, ConnectionState *>::iterator it = clientStates.find(eventFd);
                clientStates.erase(it);
            }
        }

        for (std::map<int, ConnectionState*>::iterator it = clientStates.begin(); 
            it != clientStates.end(); ) {
       
            if (it->second->hasTimedOut() || (it->second->getIsDone() && it->second->isSendingDone())) {
                Logger::getLogStream() << it->second->getEventFd() << " has timedout" << std::endl;
                std::map<int, ConnectionState*>::iterator toErase = it;
                ++it;
                delete toErase->second;
                clientStates.erase(toErase);
            } else {
                ++it;
            }
        }
    }
}

void ServerManager::start(void) {
    epollFd = epoll_create(1);
    if (epollFd == -1)
    {
        std::string errorStr = "epoll_create() failed";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    for (std::vector<Server>::iterator it = servers.begin();
        it != servers.end(); it++)
    {
        struct epoll_event ev;

        it->startServer();
        int fdSocket = it->getFdSocket();
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = new ConnectionState(fdSocket, epollFd);
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fdSocket, &ev) == -1)
        {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(fdSocket);
        }
    }
    handleConnections();
}

void ServerManager::cleanUp() {
    if (epollFd != -1)
        close(epollFd);
    for (std::vector<Server>::iterator it = servers.begin(); 
        it != servers.end(); it++) {
            if (!it->isServerUp())
                break;
            close(it->getFdSocket());
    }

    for (std::map<int, ConnectionState *>::iterator ite = clientStates.begin();
        ite != clientStates.end(); ite++) {
            delete ite->second;
        }
    
}
