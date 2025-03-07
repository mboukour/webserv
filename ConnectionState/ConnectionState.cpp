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
ConnectionState::ConnectionState(int eventFd, int epollFd) : eventFd(eventFd), epollFd(epollFd),
  readState(NO_REQUEST), bytesRead(0), request(), 
  writeState(NO_RESPONSE), sendMode(NONE), filePath(), currentPos(), stringToSend(), response(NULL),
  isDone(false) {}


void ConnectionState::handleWritable(void) {
    if (this->writeState == NO_RESPONSE)
        return;
    if (this->sendMode == FILE) {
        std::fstream fileToSend(this->filePath.c_str());
        if (!fileToSend.is_open())
            throw std::runtime_error("Could'nt open file");
    
    fileToSend.seekg(this->currentPos);
    
    // Read one chunk
        std::vector<char> buffer(READ_SIZE);
        while(true) {
            fileToSend.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = fileToSend.gcount();
        
            if (bytesRead == 0) {
                this->sendMode = NONE;
                this->writeState = NO_RESPONSE;
                return;
            }
                
            ssize_t totalSent = 0;
            while (totalSent < bytesRead) {
                ssize_t bytesSent = send(this->eventFd, buffer.data() + totalSent, bytesRead - totalSent, 0);
                if (bytesSent == -1) {
                    std::streampos filePos = fileToSend.tellg();
                    filePos -= (bytesRead - totalSent);
                    this->currentPos = filePos;
                    return;
                }
                totalSent += bytesSent;
            }
        }
    } else if (this->sendMode == STRING) {
        size_t totalSent = 0;
        while(totalSent < this->stringToSend.size()) {
            ssize_t bytesSent = send(this->eventFd, this->stringToSend.data() + totalSent, this->stringToSend.size() - totalSent, 0);
            if (bytesSent == -1) {
                this->stringToSend = this->stringToSend.substr(totalSent);
                return ;
            }
            totalSent += bytesSent;
        }
        this->sendMode = NONE;
        this->writeState = NO_RESPONSE;
        this->stringToSend.clear();
    }
}


void ConnectionState::handleReadable(std::vector<Server> &servers) {
    if (this->readState == NO_REQUEST)
        this->readState = READING_HEADERS;

    while (true) {
        std::vector<char> buffer(READ_SIZE);
        ssize_t bytesReceived = recv(this->eventFd, buffer.data(), buffer.size()- 1, 0);
        if (bytesReceived == 0) {
            DEBUG && std::cout << "Client reading side closed" << std::endl;
            this->readState = DONE_READING;

            std::cout << "WRITE: " << this->writeState << '\n';
            break;
        } else if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string bufferStr(buffer.data(), bytesReceived);
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
                this->readState = READING_BODY;
                try {
                    this->request = HttpRequest(this->requestBuffer, servers, port); // dont forget that this will throw exceptions in case of wrong http requests
                } catch (const HttpErrorException &exec) {
                    DEBUG && std::cerr << "Response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << "\n" << std::endl;
                    std::string respStr = exec.getResponseString();
                    send(this->eventFd, respStr.c_str(), respStr.size(), 0);
                    this->isDone = true;
                    DEBUG && std::cout << "Connection closed after error: " << strerror(errno) << std::endl;
                    return ;
                }
                if (this->request.getMethod() != "POST")
                    ServerManager::sendResponse(request, this->eventFd);
                if (!this->response) {
                    try {
                        this->response = new HttpResponse(this->request, this->eventFd, this->epollFd);
                    } catch (const HttpErrorException& exec) {
                        std::string respStr = exec.getResponseString();
                        send(this->eventFd, respStr.c_str(), respStr.size(), 0);
                        this->isDone = true;
                        return;
                    }
                }

                this->requestBuffer.clear();

                // std::cout << "CL: " << this->request->getContentLength() << " Body Size: " << this->request->getBodySize() << '\n';
            } else if (this->readState == READING_BODY) {
                this->request.setReqEntry(bufferStr);
                if (this->response)
                    this->response->handleNewReqEntry(this->request);
                // response->
                const std::string &contentLength = this->request.getHeader("Content-Length");
                if (contentLength == "") {
                    this->readState = NO_REQUEST;
                }
                
                if (this->request.getBodySize() == this->request.getContentLength()) { // by here the request is fully consumed and should be freed
                    std::cout <<  "END RES: CL: " << this->request.getContentLength() << " Body Size: " << this->request.getBodySize() << '\n';
                    this->readState = NO_REQUEST;
                    delete this->response;
                    this->request = HttpRequest();
                    this->response = NULL;
                } else if (this->request.getBodySize() >= this->request.getContentLength()) {
                    throw HttpErrorException(BAD_REQUEST, "BODY ISZE BIGGER THAN CL");
                    return; 
                } else {
                    std::cout << "CL: " << this->request.getContentLength() << " Body Size: " << this->request.getBodySize() << '\n'; 
                }
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

void ConnectionState::activateWriteState(const std::string &filePath, const std::streampos &currentPos) {
    this->sendMode = FILE;
    this->filePath = filePath;
    this->currentPos = currentPos;
    this->writeState = SENDING_RESPONSE;
}

void ConnectionState::activateWriteState(const std::string &stringToSend) {
    this->sendMode = STRING;
    this->stringToSend = stringToSend;
    this->writeState = SENDING_RESPONSE;
}

int ConnectionState::getEventFd(void) const {
    return this->eventFd; 
}

// bool ConnectionState::getIsRequestReady(void) const {
//     return this->isRequestReady;
// }

// bool ConnectionState::getIsRequestDone(void) const {
//     return this->isRequestDone;
// }

// HttpRequest* ConnectionState::getHttpRequest(void) const {
//     return this.request;
// }

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