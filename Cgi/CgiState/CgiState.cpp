#include "CgiState.hpp"
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <sched.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <vector>
#include <sys/wait.h>
#include "../../Utils/AllUtils/AllUtils.hpp"
#include "../../Utils/Logger/Logger.hpp"

#include "../../Server/ServerManager/ServerManager.hpp"

const int CgiState::cgiTimeout(5);

CgiState::CgiState(int cgiFd, pid_t cgiPid, int epollFd,  ClientState *client):
     cgiFd(cgiFd), epollFd(epollFd) ,cgiPid(cgiPid),
    client(client), lastActivityTime(time(NULL)),
    readMode(RAW_CHUNKED), readState(READING_HEADERS), contentSent(0), contentLength(0),
    writeState(NOT_REGISTERED), writeQueue(),
    isResponding(false), isClean(false),isDone(false) {} // we assume we have to make our own chunked unless headers specify not to :)


void CgiState::parseCgiHeaders(void) {
    std::stringstream ss(this->initBuffer);
    std::string line;
    while(std::getline(ss, line)) {
        // Handle \r\n line endings
        if (!line.empty() && line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);
            
        // Empty line indicates end of headers
        if (line.empty())
            break;
            
        // Parse key:value
        size_t pos = line.find(":");
        if (pos == std::string::npos) {
            std::cerr << "Invalid header line: " << line << std::endl;
            throw HttpErrorException(INTERNAL_SERVER_ERROR, "Invalid header found in CGI");
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        AllUtils::removeLeadingSpaces(value);
        
        // Only process the headers we care about
        if (key == "Content-Length") {
            this->readMode = CONTENT_LENGTH;
            std::stringstream clSs(value);
            clSs >> this->contentLength;
            if (clSs.fail())
                throw HttpErrorException(INTERNAL_SERVER_ERROR, "Invalid content length header in CGI");
            this->readMode = CONTENT_LENGTH;
        } else if (key == "Transfer-Encoding") {
            if (value == "chunked")
                this->readMode = READY_CHUNKED;
            else
                throw HttpErrorException(INTERNAL_SERVER_ERROR, "Unsupported Transfer-Encoding: " + value);
        }
        // Ignore all other headers
    }
}

void CgiState::setupReadMode(size_t headersPos) {

    switch (this->readMode) {
        case CONTENT_LENGTH: {
            if (this->contentSent > this->contentLength) {
                std::stringstream ss;
                ss  << "Content size: " << this->contentSent << " bigger than Content length: " << this->contentLength; 
                throw HttpErrorException(INTERNAL_SERVER_ERROR, ss.str());
            }
            else if (this->contentSent == this->contentLength) {
                cleanUpCgi();
                this->isDone = true;
            }
            break;
        }
        case RAW_CHUNKED: {
            this->currentChunk = this->initBuffer.substr(headersPos);
            this->initBuffer = this->initBuffer.substr(0, headersPos);
            this->initBuffer.insert(0, "Transfer-Encoding: chunked\r\n");
            break;
        }
        case READY_CHUNKED: {
            break;
        }
    }
}

void CgiState::setupReadMode(const std::string &bufferStr) {
    switch (this->readMode) {
        case CONTENT_LENGTH: {
            this->contentSent += bufferStr.size();
            if (this->contentSent > this->contentLength)
                throw HttpErrorException(INTERNAL_SERVER_ERROR, "Content size bigger than content length in CGI");
            else if (this->contentSent == this->contentLength) {
                cleanUpCgi();
                this->isDone = true;
            }
            this->client->activateWriteState(this->initBuffer);
            break;
        }
        case RAW_CHUNKED: {
            this->currentChunk += bufferStr;
            break;
        }
        case READY_CHUNKED: {
            break;
        }
    }
}

void CgiState::readCgi(std::string bufferStr) {
    switch (this->readState) {
        case READING_HEADERS: {
            this->initBuffer += bufferStr;
            size_t pos_crlf = bufferStr.find("\r\n\r\n");
            size_t pos_lf = bufferStr.find("\n\n");

            size_t pos;
            int delimiter_len;

            if (pos_crlf != std::string::npos) {
                pos = pos_crlf;
                delimiter_len = 4;
            } else if (pos_lf != std::string::npos) {
                pos = pos_lf;
                delimiter_len = 2;
            } else
                return ;
            parseCgiHeaders();
            this->contentSent = this->initBuffer.size() - pos - delimiter_len;
            setupReadMode(pos + delimiter_len);
            this->readState = READING_BODY;
            this->initBuffer.insert(0, "HTTP/1.1 200 OK\r\n");
            this->client->activateWriteState(this->initBuffer);
            this->isResponding = true; // init buffer will always be sent first
            break;
        }
        case READING_BODY: {
            setupReadMode(bufferStr);
        }
    }
}

void CgiState::updateLastActivity(void) {
    this->lastActivityTime = time(NULL);
}

bool CgiState::hasTimedOut(void) const {
    return time(NULL) - lastActivityTime > cgiTimeout;
}


void CgiState::notifyCgiClient(int statusCode) {
    if (!this->isResponding) {
        HttpErrorException timeout(statusCode, "Cgi script has timedout or did exit");
        std::cout << timeout.what() << std::endl;
        this->client->activateWriteState(timeout.getResponseString());
    }
}

ClientState *CgiState::getClient(void) const {
    return this->client;
}

void CgiState::sendCurrentChunk(void) {
    std::stringstream ss;
    ss << std::hex << currentChunk.size() << "\r\n" << currentChunk << "\r\n";
    this->client->activateWriteState(ss.str());
    this->currentChunk.clear();
}



void CgiState::handlecgiReadable(void) {
    while (true) {
        std::vector<char> buffer(4096);
        ssize_t bytesRead = recv(cgiFd, buffer.data(), buffer.size(), 0);
        if (bytesRead == 0) {
            if (!this->isResponding)
                notifyCgiClient(INTERNAL_SERVER_ERROR);
            else if (this->readMode == RAW_CHUNKED)
                sendCurrentChunk();
            cleanUpCgi();
            return;
        } else if (bytesRead > 0)
            readCgi(std::string(buffer.data(), bytesRead));
        else {
            if (this->readMode == RAW_CHUNKED && this->isResponding)
                sendCurrentChunk();
            updateLastActivity();
            return;
        }
    }
    updateLastActivity();
}

bool CgiState::isCgiAlive(void) const {
    pid_t res = waitpid(this->cgiPid, NULL, WNOHANG);
    if (res == -1 || res == this->cgiPid)
        return false;
    return true;
}

bool CgiState::isWritingDone(void) const {
    return this->writeQueue.empty();
}

void CgiState::activateWriteState(const std::string &toWrite) {
    std::cout << "CGI WRITE STATE" << std::endl;
    this->writeQueue.push_back(toWrite);
    if (this->writeState == NOT_REGISTERED) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = ServerManager::getEpollEvent(this->cgiFd);
        epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->cgiFd, &ev);
        this->writeState = REGISTERED;
    }
}

void CgiState::handleCgiWritable(void) {
    if (this->writeQueue.empty())
        return;
    std::cout << "WRITABLE" << std::endl;
    for (std::vector<std::string>::iterator it = this->writeQueue.begin();
        it != this->writeQueue.end(); ) {

            if (!isCgiAlive()) { // avoinding SIGPIPE
                if (!this->isResponding)
                    notifyCgiClient(INTERNAL_SERVER_ERROR);
                cleanUpCgi();
                return;
            }
            size_t totalSent = 0;
            while(totalSent < it->size()) {
                ssize_t bytesSent = send(this->cgiFd, it->c_str(), it->size(),0);
                if (bytesSent == -1) {
                    *it = it->substr(totalSent);
                    updateLastActivity();
                    return;
                }
                totalSent += bytesSent;
                std::cout << "Writing: " << bytesSent << std::endl;
            }
            it = this->writeQueue.erase(it);
        }
    // loop reached end, we just emptied the whole writeQueue
    updateLastActivity();
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = ServerManager::getEpollEvent(this->cgiFd);
    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->cgiFd, &ev);
    this->writeState = NOT_REGISTERED; 
}

bool CgiState::getIsDone(void) const {
    return this->isDone;
}

void CgiState::cleanUpCgi(void) {
    pid_t res = waitpid(cgiPid, NULL, WNOHANG);
    if (res == 0) // still running
        kill(cgiPid, SIGKILL);
    this->isClean = true;
    this->isDone = true;
}

CgiState::~CgiState() {
    if (!this->isClean)
        cleanUpCgi();
    std::cout << "Closing cgi state" << std::endl;

}