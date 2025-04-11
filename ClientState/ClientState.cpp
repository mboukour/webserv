#include "ClientState.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <ios>
#include <iosfwd>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include "../Debug/Debug.hpp"
#include "../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../Server/ServerManager/ServerManager.hpp"
#include "../Utils/Logger/Logger.hpp"
#include "../Cgi/CgiState/CgiState.hpp"

const int ClientState::keepAliveTimeout = 10;
const int ClientState::sendTimeout = 1;


ClientState::ClientState(int eventFd, int epollFd) : eventFd(eventFd), epollFd(epollFd),
  readState(NO_REQUEST), bytesRead(0), request(), requestCount(0),
  writeState(NOT_REGISTERED), sendQueue(), response(NULL), cgiState(NULL),
  lastActivityTime(time(NULL)), lastSend(time(NULL)), isKeepAlive(true),isResponding(false), isDone(false), isClean(false) {}


void ClientState::handleWritable(void) {
    if (this->sendQueue.empty()) {
        return;
    }
    updateLastActivity();
    for (std::vector<SendMe>::iterator it = this->sendQueue.begin(); it != this->sendQueue.end();)
    {
        SendMe &toSend = *it;
        if (toSend.sendMode == FILE) {
            std::fstream fileToSend(toSend.filePath.c_str());
            if (!fileToSend.is_open())
                throw std::runtime_error("Could'nt open file");

        fileToSend.seekg(toSend.currentPos);
            std::vector<char> buffer(READ_SIZE);
            while(true) {
                fileToSend.read(buffer.data(), buffer.size());
                std::streamsize bytesRead = fileToSend.gcount();
                if (bytesRead == 0) {
                    break;
                }

                ssize_t totalSent = 0;
                while (totalSent < bytesRead) {
                    this->isResponding = true;
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
                this->isResponding = true;
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
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = ServerManager::getEpollEvent(this->eventFd);
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            DEBUG && Logger::getLogStream() << "[ERROR] -> epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = NOT_REGISTERED;
    }
}


void ClientState::updateLastActivity(void) {
    this->lastActivityTime = time(NULL);
}

void ClientState::updateLastSend(void) {
    this->lastSend = time(NULL);
}

bool ClientState::hasTimedOut(void) const {
    if (request.isCgiRequest() && cgiState && cgiState->isCgiAlive())
        return false;
    return time(NULL) - lastActivityTime > keepAliveTimeout;
}

bool ClientState::getIsKeepAlive(void) const {
    return this->isKeepAlive;
}

void ClientState::resetReadState(void) {
    this->readState = NO_REQUEST;
    delete this->response;
    this->request = HttpRequest();
    this->response = NULL;
    this->requestBuffer.clear();
    this->isKeepAlive = true;
}


void ClientState::sendHttpError(const HttpErrorException &exec) {
    DEBUG && Logger::getLogStream() << "[ERROR] -> response sent with code " << exec.getStatusCode() << " Reason: " << exec.what() << std::endl;
    ServerManager::sendString(exec.getResponseString(), this->eventFd);
    resetReadState();
    this->isDone = true;
}

void ClientState::handleReadable(std::vector<Server> &servers) {
    if (this->isDone)
        return;
    if (this->readState == NO_REQUEST)
        this->readState = READING_HEADERS;

    std::vector<char> buffer(READ_SIZE);
    while (true) {
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
            if (this->readState == READING_HEADERS)
                this->requestBuffer += bufferStr;
            if (this->readState == READING_HEADERS && bufferStr.find("\r\n\r\n") != std::string::npos) {
                struct sockaddr_in addr;
                socklen_t addrLen = sizeof(addr);
                if (getsockname(this->eventFd, (struct sockaddr*)&addr, &addrLen) == -1) {
                    DEBUG && Logger::getLogStream() << "[ERROR] -> getsockname failed. Errno: " << strerror(errno) << std::endl;
                    throw HttpErrorException(INTERNAL_SERVER_ERROR, "getsockname error");
                }
                int port = ntohs(addr.sin_port);
                this->readState = READING_BODY;
                try {
                    this->requestCount++;
                    this->request = HttpRequest(this->requestBuffer, servers, port);
                    DEBUG && Logger::getLogStream() << "----------New Request----------\n" << request << "----------End Request----------" << std::endl;
                } catch (const HttpErrorException &exec) {
                    if (exec.getStatusCode() == PAYLOAD_TOO_LARGE)
                        this->isDone = true;
                    sendHttpError(exec);
                    return ;
                }
                if (this->request.getHeader("Connection") == "close") {
                    this->isKeepAlive = false;
                }
                if (!this->response) {
                    try {
                        this->response = new HttpResponse(this->request, this->eventFd, this->epollFd);
                    } catch (const HttpErrorException& exec) {
                        sendHttpError(exec);
                        return;
                    }
                }
                if (!request.isChunkedRequest() && (request.getContentLength() == 0 || request.getContentLength() == request.getBodySize())) {
                    resetReadState();
                    updateLastActivity();
                    return ;
                }
                this->requestBuffer.clear();
            } else if (this->readState == READING_BODY) {
                this->request.setReqEntry(bufferStr);
                if (request.getRequestBlock()->getIsLimited() && request.getBodySize() > request.getRequestBlock()->getMaxBodySize()) {
                    this->response->removeMultiFiles();
                    HttpErrorException exec(PAYLOAD_TOO_LARGE, request, "Payload too large");
                    sendHttpError(exec);
                    return;
                }
                if (this->response) {
                    try {
                        this->response->handleNewReqEntry(this->request);
                    } catch (const HttpErrorException &exec) {
                        this->response->removeMultiFiles();
                        sendHttpError(exec);
                        return;
                    }
                }
                if (this->response->getIsLastEntry())
                    resetReadState();
                updateLastActivity();
            }
        } else {
            return ;
        }
    }
}

ClientState::SendMe::SendMe(const std::string &filePath, const std::streampos &currentPos) {
    this->sendMode = FILE;
    this->filePath = filePath;
    this->currentPos = currentPos;
}

ClientState::SendMe::SendMe(const std::string &stringToSend) {
    this->sendMode = STRING;
    this->stringToSend = stringToSend;
}

void ClientState::SendMe::changeSend(const std::string &filePath, const std::streampos &currentPos) {
    this->sendMode = FILE;
    this->filePath = filePath;
    this->currentPos = currentPos;
}


void ClientState::SendMe::changeSend(const std::string &stringToSend) {
    this->sendMode = STRING;
    this->stringToSend = stringToSend;
}

bool ClientState::isSendingDone(void) const {
    if (this->sendQueue.empty() && time(NULL) - lastSend > sendTimeout)
        return true;
    return false;
}

void ClientState::activateWriteState(const std::string &filePath, const std::streampos &currentPos) {
    this->sendQueue.push_back(SendMe(filePath, currentPos));
    if (this->writeState == NOT_REGISTERED) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = ServerManager::getEpollEvent(this->eventFd);
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            DEBUG && Logger::getLogStream() << "[ERROR] -> epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = REGISTERED;
    }
}

void ClientState::activateWriteState(const std::string &stringToSend) {
    this->sendQueue.push_back(SendMe(stringToSend));
    if (this->writeState == NOT_REGISTERED) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = ServerManager::getEpollEvent(this->eventFd);
        if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->eventFd, &ev) == -1) {
            DEBUG && Logger::getLogStream() << "[ERROR] -> epoll_ctl failed. Errno: " << strerror(errno) << std::endl;
            close(this->eventFd);
        }
        this->writeState = REGISTERED;
    }
}

int ClientState::getEventFd(void) const {
    return this->eventFd;
}


HttpResponse * ClientState::getHttpResponse(void) const {
    return this->response;
}

const HttpRequest &ClientState::getHttpRequest(void) const {
    return this->request;
}

void ClientState::setAsDone(void) {
    this->isDone = true;
}

bool ClientState::getIsDone(void) const {
    return this->isDone;
}

bool ClientState::getIsResponding(void) const {
    return this->isResponding;
}

ClientState::~ClientState() {
    if (this->response)
        delete this->response;
}
