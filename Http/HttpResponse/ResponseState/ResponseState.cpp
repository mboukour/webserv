#include "ResponseState.hpp"
#include <iosfwd>
#include <sys/socket.h>
 #include <sys/epoll.h>
ResponseState::ResponseState() {}

ResponseState::ResponseState(const std::string &filePath, std::streampos filePos, int clientFd, int epollFd)
    : filePath(filePath), clientFd(clientFd), epollFd(epollFd) {
    this->fileToSend.open(filePath.c_str());
    if (!this->fileToSend.is_open()) { // DONT FORGET TO HANDLE IF WE CANT OPEN THE FILE
        throw std::logic_error("Error opening file");
    }
    this->fileToSend.seekg(filePos);
}


void ResponseState::continueSending(void) { // 
    std::vector<char> buffer(65536);  // 4KB buffer
    while (true) {
        fileToSend.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = fileToSend.gcount();
    
        if (bytesRead == 0) break;  // EOF
    
        // Send the data
        ssize_t totalSent = 0;
        while (totalSent < bytesRead) {
            ssize_t bytesSent = send(clientFd, buffer.data() + totalSent, bytesRead - totalSent, 0);
            if (bytesSent == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    std::streampos filePos = fileToSend.tellg();
                    filePos -= (bytesRead - totalSent);
                    struct epoll_event ev;
                    ResponseState *state = new ResponseState(this->filePath, filePos, this->clientFd, this->epollFd);
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = this->clientFd;
                    ev.data.ptr = state;
                    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->clientFd, &ev);
                    return;
                } else { // implement better error handling later
                    perror("Error: ");
                    return ; 
                }
            }
            totalSent += bytesSent;
        }
    }
    // restore to monitor for reading only once chunking is done
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = this->clientFd;
    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->clientFd, &ev);
}

ResponseState::~ResponseState() {}