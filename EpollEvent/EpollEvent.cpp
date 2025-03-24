#include "EpollEvent.hpp"
#include <cmath>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <sys/epoll.h>

EpollEvent::EpollEvent(int eventFd, int epollFd, EventType eventType): // for servers and clients
    eventFd(eventFd), epollFd(epollFd), eventType(eventType), lastActivityTime(time(NULL)) {
        if (this->eventType == CGI_READ)
            throw std::logic_error("Invalid EpollEvent constructor for init CGI");
        if (this->eventType == CLIENT_CONNECTION)
            this->eventData.clientState = new ClientState(this->eventFd, this->epollFd);
        else if (this->eventType == SERVER_SOCKET)
            this->eventData.clientState = NULL;
        std::cout << "EventType for " << eventFd << " is " << eventType << " " << this << std::endl;
}

EpollEvent::EpollEvent(int cgiRead, int cgiPid, int epollFd , ClientState *clientToSend): 
    eventFd(cgiRead), epollFd(epollFd), eventType(CGI_READ), lastActivityTime(time(NULL)) {
        this->eventData.cgiState = new CgiState(cgiRead, cgiPid, clientToSend);
}

// EpollEvent::EpollEvent(const EpollEvent& other): eventFd(other.eventFd), epollFd(other.epollFd), eventType(other.eventType) {
//     throw std::logic_error("NO COPY ALLOWED");
// }

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
        throw std::logic_error("Trying to get client state in non client event");
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
    else // this->eventType == CGI_READ
        return this->eventData.cgiState->getIsDone();
}

EpollEvent::~EpollEvent() {
    std::cout << "Closing event " << eventFd << std::endl;
    epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->eventFd, NULL);
    close (this->eventFd);
}

// 94290722878528 
// 94290722874576