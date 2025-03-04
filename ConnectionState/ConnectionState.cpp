#include "ConnectionState.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "../Debug/Debug.hpp"
ConnectionState::ConnectionState(int eventFd, int epollFd) : eventFd(eventFd), epollFd(epollFd),
  readState(NO_REQUEST), bytesRead(0), request(NULL), isRequestReady(false), isRequestDone(false),
  writeState(NO_RESPONSE), responseState(NULL),
  isDone(false), lastEvent(0) {}


void ConnectionState::handleWritable(void) {
    if (!this->responseState)
        throw std::logic_error("NULL responseState");
    if (this->writeState == NO_RESPONSE)
        this->writeState = SENDING_RESPONSE;
    ResponseState *nextRespState = this->responseState->continueSending();
    delete this->responseState;
    if (!nextRespState) {
        this->writeState = NO_RESPONSE;
    }
    this->responseState = nextRespState;
    updateEpollEvents();
    return ;
}

void ConnectionState::handleReadable(std::vector<Server> &servers) {
    if (this->readState == NO_REQUEST)
        this->readState = READING_HEADERS;

    char buffer[1024];
    ssize_t bytesReceived = recv(this->eventFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0) {
        DEBUG && std::cout << "Client reading side closed" << std::endl;
        this->readState = DONE_READING;
        std::cout << "WRITE: " << this->writeState << '\n';
        
    } else if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::string bufferStr(buffer);
        this->requestBuffer+= bufferStr;
        if (this->readState == READING_HEADERS && bufferStr.find("\r\n\r\n") != std::string::npos) {
            struct sockaddr_in addr;
            socklen_t addrLen = sizeof(addr);
            if (getsockname(this->eventFd, (struct sockaddr*)&addr, &addrLen) == -1) {
                std::cerr << "Error: getsockname failed. Errno: " << strerror(errno) << std::endl;
                close(this->eventFd);
                epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->eventFd, NULL);
                throw std::logic_error("getsockname error");
            }
            int port = ntohs(addr.sin_port);
            // std::cout << "Found Headers: " <<  bufferStr << '\n';
            this->readState = READING_BODY;
            this->request = new HttpRequest(this->requestBuffer, servers, port); // dont forget that this will throw exceptions in case of wrong http requests
            std::cout << *this->request;
            // this->requestBuffer = "";
            this->requestBuffer.clear();
            if (this->request->getMethod() != "POST")
                this->isRequestReady = true;
            // the readState should be reading body in case of post??
        } else if (this->readState == READING_BODY) {
            // if (this->request->getMethod() != "POST") { // consuming the not needed bodies
            //     return ; 
            // }
            this->request->appendToBody(bufferStr);
            const std::string &contentLength = this->request->getHeader("Content-Length");
            if (contentLength == "") {
                this->readState = NO_REQUEST;
                this->isRequestDone = true;
                updateEpollEvents();
            }
            std::stringstream ss(contentLength);
            size_t size;
            ss >> size;
            if (this->request->getBodySize() >= size) { // by here the request is fully consumed and should be freed
                this->readState = NO_REQUEST;
                this->isRequestReady = true;
                this->isRequestDone = true;
                updateEpollEvents();

            }
        }
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return ;
        else
            std::cerr << "Error: " << strerror(errno) << std::endl;
    }
    updateEpollEvents();
}

void ConnectionState::setResponseState(ResponseState *responseState) {
    this->responseState = responseState;
}

int ConnectionState::getEventFd(void) const {
    return this->eventFd; 
}

bool ConnectionState::getIsRequestReady(void) const {
    return this->isRequestReady;
}

bool ConnectionState::getIsRequestDone(void) const {
    return this->isRequestDone;
}

HttpRequest* ConnectionState::getHttpRequest(void) const {
    return this->request;
}

bool ConnectionState::getIsDone(void) const {
    return this->isDone;
}

void ConnectionState::updateEpollEvents(void) { // will need to throw an exception of there is no need for this connection state from now on
    uint32_t events = 0;

    if (this->readState != DONE_READING)
        events |= EPOLLIN;
    if (this->writeState != NO_RESPONSE)
        events |= EPOLLOUT;

    if (events == 0) {
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->eventFd, NULL);
        close(this->eventFd);
        this->isDone = true;
        std::cout << "NOTHING :D\n";
        return ;
    } else if (events == this->lastEvent) // no need to reset
        return ;
    this->lastEvent = events;
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = this;
    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev);
}

ConnectionState::~ConnectionState() {
    if (this->request) {
        delete this->request;
    }
    if (this->responseState) {
        delete this->responseState;
    }
    close(this->eventFd);
}