#include <cstddef>
#include <cstdlib>
#include <stdexcept>
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
#include "../../ClientState/ClientState.hpp"
#include "../../Utils/Logger/Logger.hpp"
int ServerManager::epollFd = -1;
std::vector<Server> ServerManager::servers;
std::map<int, EpollEvent*> ServerManager::eventStates;

ServerManager::ServerManager() {}

void ServerManager::initialize(std::vector<Server> &serversList) {
    epollFd = -1;
    servers = serversList;
}

void ServerManager::sendString(const std::string &str, int clientFd) {
    // EpollEvent *
    ClientState *state = getClientState(clientFd);
    state->activateWriteState(str);
}

void ServerManager::sendFile(const std::string &filePath, int clientFd) {
    ClientState *state = getClientState(clientFd);
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
    EpollEvent* clientEvent = new EpollEvent(clientFd, epollFd, EpollEvent::CLIENT_CONNECTION);
    ev.data.ptr = clientEvent;
    eventStates[clientFd] = clientEvent;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1)
    {
        std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
        close(clientFd);
    }
}

ClientState *ServerManager::getClientState(int clientFd) {
    try {
        return eventStates.at(clientFd)->getClientState();
    } catch (const std::out_of_range &ex) {
        std::cout << "Client state out of range" << std::endl;
        return NULL;
    }
}

EpollEvent *ServerManager::getEpollEvent(int clientFd) {
    // try {
        return eventStates.at(clientFd);
    // } catch (const std::out_of_range &ex) {
    //     std::cout << "Epoll event out of range: " << clientFd << std::endl;
    //     return NULL;
    // }
}

void ServerManager::registerEpollEvent(int fd, EpollEvent *event) {
    eventStates[fd] = event;
}

void ServerManager::cgiEpoll(EpollEvent *epollEvent, struct epoll_event &event) {
    CgiState *state = epollEvent->getCgiState();
    if (event.events & EPOLLIN) // we only register cgi for readable, we will never write to it
        state->handlecgiReadable();
    if (event.events & EPOLLOUT) {
        state->handleCgiWritable();
    }
}

void ServerManager::clientServerEpoll(EpollEvent *epollEvent, struct epoll_event &event) {
    const int eventFd = epollEvent->getEventFd();
    if (epollEvent->getEventType() == EpollEvent::SERVER_SOCKET) {
        if (event.events & EPOLLIN)
            acceptConnections(eventFd);
        return;
    }
    ClientState *state = epollEvent->getClientState();
    if (event.events & EPOLLIN)
        state->handleReadable(servers);
    if (event.events & EPOLLOUT)
        state->handleWritable();
}


void ServerManager::removeCgiAfterClient(ClientState *client) {
    for (std::map<int, EpollEvent *>::iterator it = eventStates.begin();
        it != eventStates.end(); ) {
            std::map<int, EpollEvent*>::iterator toErase = it;
            it++;
            if (toErase->second->getEventType() == EpollEvent::CGI_READ) {
                CgiState *cgiState = toErase->second->getCgiState();
                if (cgiState->getClient() == client) {
                    cgiState->cleanUpCgi();
                    delete toErase->second;
                    eventStates.erase(toErase);
                }
            }
        }
}

bool ServerManager::checkIfDone(EpollEvent *event) {
    const int eventType = event->getEventType();
    switch (eventType) {
        case EpollEvent::SERVER_SOCKET: { // we skip it, therefore not posi
            throw std::logic_error("Can't check if server socket has timedout");
        }
        case EpollEvent::CGI_READ: {
            CgiState *cgiState = event->getCgiState();
            if ((event->getIsDone() || event->hasTimedOut()) && cgiState->isWritingDone()) {
                cgiState->notifyCgiClient(GATEWAY_TIMEOUT);
                return true;
            }
            return false;
        }
        case EpollEvent::CLIENT_CONNECTION: {
            ClientState *clientState = event->getClientState();
            if ((event->getIsDone() || event->hasTimedOut()) && clientState->isSendingDone()) {
                removeCgiAfterClient(clientState);
                return true;
            }
            // if (clientState->isSendingDone()) {
            //     removeCgiAfterClient(clientState);
            //     return true;
            // }
            return false; // sending needs to be set as done
        }
    }
    return false;
    // if (!event->getIsDone())
    //     return false;
    // if (event->getEventType() == EpollEvent::CGI_READ) // done cgi read
    //     return true;
    // if (event->getEventType() == EpollEvent::CLIENT_CONNECTION) {
    //     ClientState *state = event->getClientState();
    //     if (state->isSendingDone())
    //         return true;
    // }
    // return false;
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
            // ClientState *state = static_cast<ClientState *>(events[i].data.ptr);
            EpollEvent *epollEvent = static_cast<EpollEvent *>(events[i].data.ptr);
            const int eventType = epollEvent->getEventType();
            if (eventType == EpollEvent::SERVER_SOCKET || eventType == EpollEvent::CLIENT_CONNECTION)
                clientServerEpoll(epollEvent, events[i]);
            else // CGI_READ
                cgiEpoll(epollEvent, events[i]);
        }

        for (std::map<int, EpollEvent*>::iterator it = eventStates.begin(); 
            it != eventStates.end(); ) {
            if (it->second->getEventType() == EpollEvent::SERVER_SOCKET) {
                ++it;
                continue;
            }
            if (checkIfDone(it->second)) {
                
                Logger::getLogStream() << it->second->getEventFd() << " has timedout or is done" << std::endl;
                std::map<int, EpollEvent*>::iterator toErase = it;
                ++it;
                delete toErase->second;
                eventStates.erase(toErase);
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
        // ev.data.ptr = new ClientState(fdSocket, epollFd);
        EpollEvent *serverEvent = new EpollEvent(fdSocket, epollFd, EpollEvent::SERVER_SOCKET);
        eventStates[fdSocket] = serverEvent;
        ev.data.ptr = serverEvent;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fdSocket, &ev) == -1)
        {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(fdSocket);
        }
    }
    handleConnections();
}

void ServerManager::cleanUp() {
    // for (std::map<int, EpollEvent *>::iterator ite = eventStates.begin();
    // ite != eventStates.end(); ite++) {
    //     delete ite->second;
    // }
    
    // if (epollFd != -1)
    //     close(epollFd);
}
