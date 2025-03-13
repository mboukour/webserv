#include "ConnectionState.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ios>
#include <iosfwd>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "../Debug/Debug.hpp"
#include "../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../Server/ServerManager/ServerManager.hpp"
#include "../Utils/Logger/Logger.hpp"

const int ConnectionState::keepAliveTimeout = 10; // in seconds

ConnectionState::ConnectionState(int eventFd, int epollFd) : eventFd(eventFd), epollFd(epollFd),
  readState(NO_REQUEST), bytesRead(0), request(), 
  writeState(NOT_REGISTERED), sendQueue(), response(NULL), 
  lastActivityTime(time(NULL)), isKeepAlive(true), isDone(false) {}


void ConnectionState::handleWritable(void) {
    if (this->sendQueue.empty()) {
        Logger::getLogStream() << "Nope, nothing to send for " << this->eventFd << std::endl;
        return;
    }
    Logger::getLogStream() << "Handling writable for " << this->eventFd << std::endl;
    updateLastActivity();
    for (std::vector<SendMe>::iterator it = this->sendQueue.begin(); it != this->sendQueue.end();)
    {
        SendMe &toSend = *it;
        if (toSend.sendMode == FILE) {
            std::fstream fileToSend(toSend.filePath.c_str());
            if (!fileToSend.is_open())
                throw std::runtime_error("Could'nt open file");
        
        fileToSend.seekg(toSend.currentPos);
        
        // Read one chunk
            std::vector<char> buffer(READ_SIZE);
            while(true) {
                fileToSend.read(buffer.data(), buffer.size());
                std::streamsize bytesRead = fileToSend.gcount();
                if (bytesRead == 0) {
                    break;
                }
                    
                ssize_t totalSent = 0;
                while (totalSent < bytesRead) {
                    ssize_t bytesSent = send(this->eventFd, buffer.data() + totalSent, bytesRead - totalSent, 0);
                    if (bytesSent == -1) {
                        std::streampos filePos = fileToSend.tellg();
                        filePos -= (bytesRead - totalSent);
                        it->changeSend(toSend.filePath, filePos);
                        return;
                    }
                    totalSent += bytesSent;
                }
            }
        } else if (toSend.sendMode == STRING) {
            size_t totalSent = 0;
            while(totalSent < toSend.stringToSend.size()) {
                ssize_t bytesSent = send(this->eventFd, toSend.stringToSend.data() + totalSent, toSend.stringToSend.size() - totalSent, 0);
                if (bytesSent == -1) {
                    it->changeSend(toSend.stringToSend.substr(totalSent));
                    return ;
                }
                totalSent += bytesSent;
            }
        }
        it = this->sendQueue.erase(it);
    }
    if (this->sendQueue.empty()) {
        Logger::getLogStream() << "Send queue is empty for " << this->eventFd << std::endl;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = this;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = NOT_REGISTERED;
    }

}


void ConnectionState::updateLastActivity(void) {
    this->lastActivityTime = time(NULL);
}

bool ConnectionState::hasTimedOut(void) const {
    return time(NULL) - lastActivityTime > keepAliveTimeout;
}

bool ConnectionState::getIsKeepAlive(void) const {
    return this->isKeepAlive;
}

void ConnectionState::resetReadState(void) {
    this->readState = NO_REQUEST;
    delete this->response;
    this->request = HttpRequest();
    this->response = NULL;
    this->requestBuffer.clear();
    this->isKeepAlive = true;
}


bool ConnectionState::isChunkedRequestComplete(void) const {
    size_t posOfChunkEnd = this->requestBuffer.find("0\r\n\r\n");
    if (posOfChunkEnd == std::string::npos)
        return false;
    return true;
}

void ConnectionState::handleReadable(std::vector<Server> &servers) {
    updateLastActivity();
    if (this->readState == NO_REQUEST)
        this->readState = READING_HEADERS;

    while (true) {
        std::vector<char> buffer(READ_SIZE);
        ssize_t bytesReceived = recv(this->eventFd, buffer.data(), buffer.size()- 1, 0);
        if (bytesReceived == 0) {
            this->readState = NO_REQUEST;
            delete this->response;
            this->request = HttpRequest();
            this->response = NULL;
            this->isDone = true;
            return ;
        } else if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string bufferStr(buffer.data(), bytesReceived);
            this->requestBuffer+= bufferStr;
            if (this->readState == READING_HEADERS && bufferStr.find("\r\n\r\n") != std::string::npos) {
                struct sockaddr_in addr;
                socklen_t addrLen = sizeof(addr);
                if (getsockname(this->eventFd, (struct sockaddr*)&addr, &addrLen) == -1) {
                    std::cerr << "Error: getsockname failed. Errno: " << strerror(errno) << std::endl;
                    throw HttpErrorException(500, "getsockname error");
                }
                int port = ntohs(addr.sin_port);
                this->readState = READING_BODY;
                std::cout << MAGENTA << "Changed state\n" << RESET;
                try {
                    this->request = HttpRequest(this->requestBuffer, servers, port); // dont forget that this will throw exceptions in case of wrong http requests
                    std::cout << "New Req: " << this->request;
                } catch (const HttpErrorException &exec) {
                    DEBUG && std::cerr << "Response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << "\n" << std::endl;
                    std::string respStr = exec.getResponseString();
                    ServerManager::sendString(respStr, this->eventFd);
                    resetReadState();
                    DEBUG && std::cout << "Connection closed after error: " << strerror(errno) << std::endl;
                    return ;
                }
                if (this->request.getHeader("Connection") == "close") {
                    this->isKeepAlive = false;
                }
                if (!this->response) {
                    try {
                        this->response = new HttpResponse(this->request, this->eventFd, this->epollFd);
                    } catch (const HttpErrorException& exec) {
                        std::string respStr = exec.getResponseString();
                        resetReadState();
                        ServerManager::sendString(respStr, this->eventFd);
                        return;
                    }
                }
                if (!request.isChunkedRequest() && (request.getContentLength() == 0 || request.getContentLength() == request.getBodySize())) {
                    resetReadState();
                    return ;
                }
                this->requestBuffer.clear();
            } else if (this->readState == READING_BODY) {
                this->request.setReqEntry(bufferStr);
                bool isLastEntry;
                if (!request.isChunkedRequest())
                    isLastEntry = this->request.getBodySize() == this->request.getContentLength();
                else
                    isLastEntry = isChunkedRequestComplete();
                if (this->response) {
                    if (isLastEntry)
                        this->response->setAsLastEntry();
                    this->response->handleNewReqEntry(this->request);
                }
                if (isLastEntry)
                    resetReadState();

                // } else if (this->request.getBodySize() > this->request.getContentLength()) {
                //     std::cout << "BODY: " << this->request.getBodySize() << '\n';
                //     throw HttpErrorException(BAD_REQUEST, "BODY SIZE BIGGER THAN CL");
                // } else {
                //     std::cout << "CL: " << this->request.getContentLength() << " Body Size: " << this->request.getBodySize() << '\n'; 
                // }
            }
        } else {
            //error state
            return ;
        }
    }
}

// void ConnectionState::setResponseState(ResponseState *responseState) {
//     this->responseState = responseState;
// }

ConnectionState::SendMe::SendMe(const std::string &filePath, const std::streampos &currentPos) {
    this->sendMode = FILE;
    this->filePath = filePath;
    this->currentPos = currentPos;
}

ConnectionState::SendMe::SendMe(const std::string &stringToSend) {
    this->sendMode = STRING;
    this->stringToSend = stringToSend;
}


void ConnectionState::activateWriteState(const std::string &filePath, const std::streampos &currentPos) {
    Logger::getLogStream() << "Activating write state for " << this->eventFd << std::endl;
    this->sendQueue.push_back(SendMe(filePath, currentPos));
    if (this->writeState == NOT_REGISTERED) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = this;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = REGISTERED;
    }
    // this->sendMode = FILE;
    // this->filePath = filePath;
    // this->currentPos = currentPos;
}

void ConnectionState::activateWriteState(const std::string &stringToSend) {
    Logger::getLogStream() << "Activating write state for " << this->eventFd << std::endl;

    this->sendQueue.push_back(SendMe(stringToSend));
    if (this->writeState == NOT_REGISTERED) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = this;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            std::cerr << "Error: epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = REGISTERED;
    }
    // this->sendMode = STRING;
    // this->stringToSend = stringToSend;
}

int ConnectionState::getEventFd(void) const {
    return this->eventFd; 
}


HttpResponse * ConnectionState::getHttpResponse(void) const {
    return this->response;
}

bool ConnectionState::getIsDone(void) const {
    return this->isDone;
}

ConnectionState::~ConnectionState() {
    if (this->response)
        delete this->response;
    epoll_ctl(epollFd, EPOLL_CTL_DEL, this->eventFd, NULL);
    close(this->eventFd);
}