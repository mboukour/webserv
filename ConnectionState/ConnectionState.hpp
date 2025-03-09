#ifndef CONNECTIONSTATE_HPP
#define CONNECTIONSTATE_HPP

#include <cstddef>
#include <ctime>
#include <fstream>
#include <iosfwd>
#include <sys/types.h>
#include <vector>
#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/HttpResponse.hpp"

#define READ_SIZE 65536

class ConnectionState {
    private: 
        static const int keepAliveTimeout;
        const int eventFd;
        const int epollFd;
        enum ReadState {NO_REQUEST, READING_HEADERS, READING_BODY, DONE_READING};
        ReadState readState;
        ssize_t bytesRead;
        std::string requestBuffer;
        HttpRequest request;
        enum WriteState {NO_RESPONSE, SENDING_RESPONSE};
        WriteState writeState;
        enum SendMode {NONE, STRING, FILE};
        SendMode sendMode;
        std::string filePath;
        std::streampos currentPos;
        std::string stringToSend;
        HttpResponse *response;
        time_t lastActivityTime;
        bool isKeepAlive;
        bool isDone;

        void resetReadState(void);
        void updateLastActivity(void);
        public:
        ConnectionState(int clientFd, int epollFd);
        // HttpRequest *getHttpRequest(void) const;
        HttpResponse *getHttpResponse(void) const;
        int getEventFd(void) const;
        bool getIsDone(void) const;
        void handleReadable(std::vector<Server> &servers);
        void activateWriteState(const std::string &filePath, const std::streampos &currentPos);
        void activateWriteState(const std::string &stringToSend);
        void handleWritable(void);
        bool hasTimedOut(void) const;
        bool getIsKeepAlive(void) const;
        ~ConnectionState();
};




#endif