#include "EpollEvent.hpp"
#include <cmath>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <sys/epoll.h>

EpollEvent::EpollEvent(int eventFd, int epollFd, EventType eventType):
    eventFd(eventFd), epollFd(epollFd), eventType(eventType) {
        if (this->eventType == CGI_READ)
            throw std::logic_error("Invalid EpollEvent constructor for init CGI");
        if (this->eventType == CLIENT_CONNECTION)
            this->eventData.clientState = new ClientState(this->eventFd, this->epollFd);
        else if (this->eventType == SERVER_SOCKET)
            this->eventData.clientState = NULL;
}

EpollEvent::EpollEvent(int cgiFd, int cgiPid, int epollFd , ClientState *clientToSend): 
    eventFd(cgiFd), epollFd(epollFd), eventType(CGI_READ) {
        this->eventData.cgiState = new CgiState(cgiFd, cgiPid, epollFd, clientToSend);
}


ClientState *EpollEvent::getClientState(void) const {
    if (this->eventType != CLIENT_CONNECTION) {
        std::stringstream ss;
        ss << eventType << " doesnt have client state";
        throw std::logic_error(ss.str());
    }
    return this->eventData.clientState;
}

CgiState *EpollEvent::getCgiState(void) const {
    if (this->eventType != CGI_READ)
        throw std::logic_error("Trying to get cgi state in non cgi event");
    return this->eventData.cgiState;
}

int EpollEvent::getEventFd(void) const {
    return this->eventFd;
}

EpollEvent::EventType EpollEvent::getEventType(void) const {
    return this->eventType;
}

bool EpollEvent::getIsDone(void) const {
    if (this->eventType == SERVER_SOCKET)
        throw std::logic_error("Server can't be done");
    else if (this->eventType == CLIENT_CONNECTION)
        return this->eventData.clientState->getIsDone();
    else
        return this->eventData.cgiState->getIsDone();
}

bool EpollEvent::hasTimedOut(void) const {
    if (this->eventType == SERVER_SOCKET)
        throw std::logic_error("Server sockets can't time out");
    else if (this->eventType == CLIENT_CONNECTION)
        return this->eventData.clientState->hasTimedOut();
    else
        return this->eventData.cgiState->hasTimedOut();
}

EpollEvent::~EpollEvent() {
    if (this->eventType == CLIENT_CONNECTION)
        delete this->eventData.clientState;
    else if (this->eventType == CGI_READ)
        delete this->eventData.cgiState;
    epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->eventFd, NULL);
    close (this->eventFd);
}
