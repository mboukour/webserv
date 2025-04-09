#include "CgiState.hpp"
#include <cctype>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iomanip>
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
    isResponding(false), isClean(false),isDone(false), cgiChunkState(CH_START), cgiOffset(0),
    cgi_chunk_size(""), cgi_remaining_chunk_size(0) {} // we assume we have to make our own chunked unless headers specify not to :)


void CgiState::parseCgiHeaders(void) {
    std::stringstream ss(this->initBuffer);
    std::string line;
    while(std::getline(ss, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);

        if (line.empty())
            break;

        size_t pos = line.find(":");
        if (pos == std::string::npos) {
            std::cerr << "Invalid header line: " << line << std::endl;
            throw HttpErrorException(INTERNAL_SERVER_ERROR, "Invalid header found in CGI");
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        AllUtils::removeLeadingSpaces(value);
        for (std::string::iterator it = key.begin(); it != key.end(); it++)
            *it = std::tolower(*it);
        if (key == "content-length") {
            this->readMode = CONTENT_LENGTH;
            std::stringstream clSs(value);
            clSs >> this->contentLength;
            if (clSs.fail())
                throw HttpErrorException(INTERNAL_SERVER_ERROR, "Invalid content length header in CGI");
            this->readMode = CONTENT_LENGTH;
        } else if (key == "transfer-encoding") {
            if (value == "chunked")
                this->readMode = READY_CHUNKED;
            else
                throw HttpErrorException(INTERNAL_SERVER_ERROR, "Unsupported Transfer-Encoding: " + value);
        }
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
            if (hasChunkEnded(this->initBuffer)) {
                cleanUpCgi();
                this->isDone = true;
            }
            break;
        }
    }
}

bool CgiState::hasChunkEnded(const std::string &toCheck) {
	bool processing = true;
	size_t curr_pos = 0;
	this->cgiOffset = toCheck.length();
	while (processing)
	{
		switch (this->cgiChunkState){
			case CH_START:
				this->cgiChunkState = CH_SIZE;
				break ;
			case CH_SIZE:{
				size_t end;
				if (this->cgiPendingCRLF) {
					this->cgiPendingCRLF = false;
					curr_pos++;
				}
				for (end = curr_pos; end < this->cgiOffset; end++){
					if ((toCheck[end] == '\r' && toCheck[end + 1] == '\n') || toCheck[end] == '\n') {
						this->cgiChunkState = CH_DATA;
						break;
					}
				}
				this->cgi_chunk_size += toCheck.substr(curr_pos, end - curr_pos);
				if (this->cgi_chunk_size.size() > 20)
                    throw HttpErrorException(INTERNAL_SERVER_ERROR, "Size bigger than 20");
				curr_pos = end;
				if (this->cgiChunkState == CH_DATA){
					std::stringstream ss(this->cgi_chunk_size);
					this->cgi_chunk_size = "";
					if ((toCheck[curr_pos] == '\r' && toCheck[curr_pos + 1] == '\n'))
						curr_pos += 2;
					else if (toCheck[curr_pos] == '\n')
						curr_pos++;
					ss >> std::setbase(16) >> this->cgi_remaining_chunk_size;
					if (this->cgi_remaining_chunk_size == 0)
						this->cgiChunkState = CH_COMPLETE;
				}
				if (curr_pos >= this->cgiOffset)
					processing = false;
				break;
			}
			case CH_DATA:
			{
				size_t ch_size = this->cgiOffset - curr_pos;
				processing = false;
				if (this->cgi_remaining_chunk_size <= ch_size) {
					ch_size = this->cgi_remaining_chunk_size;
					processing = true;
					this->cgiChunkState = CH_TRAILER;
				}
				this->cgi_remaining_chunk_size -= ch_size;
				if (!this->cgi_remaining_chunk_size)
					this->cgiChunkState = CH_TRAILER;
				curr_pos += ch_size;
				if (curr_pos >= this->cgiOffset)
					processing = false;
				break;
			}
			case CH_TRAILER:
				if ((toCheck[curr_pos] == '\r' && toCheck[curr_pos + 1] == '\n')) {
					if (curr_pos + 2 > this->cgiOffset)
						return false;
					curr_pos += 2;
				}
				else if (toCheck[this->cgiOffset - 1] == '\r') {
					this->cgiPendingCRLF = true;
					this->cgiChunkState = CH_SIZE;
					return false;
				}
				this->cgiChunkState = CH_SIZE;
				break;
			case CH_COMPLETE:
				return true;
			default:
				break;
		}
	}
    return false;
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
        if (hasChunkEnded(bufferStr)) {
            cleanUpCgi();
            this->isDone = true;
        }
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
    std::vector<char> buffer(READ_SIZE);
    while (true) {
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
