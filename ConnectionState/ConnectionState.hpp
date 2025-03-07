#ifndef CONNECTIONSTATE_HPP
#define CONNECTIONSTATE_HPP

#include <cstddef>
#include <fstream>
#include <iosfwd>
#include <sys/types.h>
#include <vector>
#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/HttpResponse.hpp"

#define READ_SIZE 65536

class ConnectionState {
    private:    
        int eventFd;
        int epollFd;
        enum ReadState {NO_REQUEST, READING_HEADERS, READING_BODY, DONE_READING};
        ReadState readState;
        ssize_t bytesRead;
        std::string requestBuffer;
        HttpRequest request;
        // bool isRequestReady;
        // bool isRequestDone;

        enum WriteState {NO_RESPONSE, SENDING_RESPONSE};
        WriteState writeState;
        enum SendMode {NONE, STRING, FILE};
        SendMode sendMode;
        std::string filePath;
        std::streampos currentPos;
        std::string stringToSend;
        HttpResponse *response;
        bool isDone;

    public:
        ConnectionState(int clientFd, int epollFd);
        // HttpRequest *getHttpRequest(void) const;
        HttpResponse *getHttpResponse(void) const;
        int getEventFd(void) const;
        bool getIsDone(void) const;
        bool getIsRequestReady(void) const;
        bool getIsRequestDone(void) const;
        void handleReadable(std::vector<Server> &servers);
        void activateWriteState(const std::string &filePath, const std::streampos &currentPos);
        void activateWriteState(const std::string &stringToSend);
        void handleWritable(void);
        ~ConnectionState();
};




#endif