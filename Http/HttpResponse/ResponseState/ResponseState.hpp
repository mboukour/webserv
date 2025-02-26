#ifndef RESPONSESTATE_HPP
#define RESPONSESTATE_HPP


#include <fstream>
#include <iosfwd>
#include <sys/types.h>
#include <vector>
class ResponseState {
    private:
        std::string filePath;
        std::fstream fileToSend;
        int clientFd;
        int epollFd;

        ResponseState();
    public:
        ResponseState(const std::string &filePath, std::streampos filePos, int clientFd, int epollFd);
        // ResponseState(const ResponseState& other);
        // ResponseState& operator=(const ResponseState& other);
        ~ResponseState();
        void continueSending(void);
};


#endif