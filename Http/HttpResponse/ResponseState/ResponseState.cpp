#include "ResponseState.hpp"
#include <iosfwd>
#include <sys/socket.h>
 #include <sys/epoll.h>
#include <sys/types.h>
ResponseState::ResponseState() {}

ResponseState::ResponseState(const std::string &filePath, std::streampos filePos, int clientFd, int epollFd)
    : filePath(filePath), clientFd(clientFd), epollFd(epollFd) {
    this->fileToSend.open(filePath.c_str());
    if (!this->fileToSend.is_open()) { // DONT FORGET TO HANDLE IF WE CANT OPEN THE FILE
        throw std::logic_error("Error opening file");
    }
    this->fileToSend.seekg(filePos);
}


ResponseState* ResponseState::continueSending(void) { // 
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
                    ResponseState *state = new ResponseState(this->filePath, filePos, this->clientFd, this->epollFd);
                    return state;
                } else { // implement better error handling later
                    perror("Error: "); // probably should throw exception here
                    return NULL; 
                }
            }
            totalSent += bytesSent;
        }
    }
    // restore to monitor for reading only once chunking is done
    return NULL; // NULL if done
}


// ResponseState &ResponseState::operator=(ResponseState &other) {
//     if (this == &other)
//         return *this;
//     this->filePath = other.filePath;
//     this->clientFd = other.clientFd;
//     this->epollFd = other.epollFd;
//     this->fileToSend.open(filePath.c_str());
//     if (!this->fileToSend.is_open()) { // DONT FORGET TO HANDLE IF WE CANT OPEN THE FILE
//         throw std::logic_error("Error opening file");
//     }
//     const std::streampos pos = other.fileToSend.tellg();
//     this->fileToSend.seekg(pos);
//     return *this;
// }

ResponseState::~ResponseState() {
    if (fileToSend.is_open()) {
        fileToSend.close();
    }
}