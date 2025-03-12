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

int ServerManager::epollFd = -1;
std::vector<Server> ServerManager::servers;
std::map<int, ConnectionState*> ServerManager::clientStates;

ServerManager::ServerManager() {}

void ServerManager::initialize(std::vector<Server> &serversList) {
    servers = serversList;
}

void ServerManager::sendString(const std::string &str, int clientFd) {
    // size_t totalSent = 0;
    // while (totalSent < str.size()) {
    //     ssize_t bytesSent = send(clientFd, str.c_str(), str.size(), 0);
    //     if (bytesSent == -1) {
    //         ConnectionState * state = clientStates.at(clientFd);
    //         std::string newStr = str.substr(totalSent);
    //         state->activateWriteState(newStr);
    //         return ;
    //     }
    //     totalSent += bytesSent;
    // }
    // std::cout << "SENDING :)" << std::endl;
    ConnectionState *state = getConnectionState(clientFd);
    state->activateWriteState(str);
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
                    ServerManager::sendString(respStr, clientFd);
                    return ;
                }
            } catch (std::bad_cast &) {}
            HttpResponse response(request, clientFd, epollFd); // this needs more work-> matching is done via port + server name, we need a server choosing algorithm, send not found if we cant find it!!!
        }
    } catch (const HttpErrorException &exec) {
        DEBUG && std::cerr << "Response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << "\n" << std::endl;
        std::string respStr = exec.getResponseString();
        ServerManager::sendString(respStr, clientFd);
        std::cout << "clientFd: " << clientFd <<std::endl;
        epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
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
    ev.events = EPOLLIN | EPOLLOUT;
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
                // std::cout << eventFd << " is writable" << std::endl;
                state->handleWritable();
            }
            if (state->getIsDone() || !state->getIsKeepAlive()) {
                delete state;
                std::map<int, ConnectionState *>::iterator it = clientStates.find(eventFd);
                clientStates.erase(it);
            }
        }

        for (std::map<int, ConnectionState*>::iterator it = clientStates.begin(); 
            it != clientStates.end(); ) {
       
            if (it->second->hasTimedOut()) {
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
        ev.events = EPOLLIN;
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
