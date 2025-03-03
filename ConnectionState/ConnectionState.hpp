#ifndef CONNECTIONSTATE_HPP
#define CONNECTIONSTATE_HPP

#include <cstddef>
#include <fstream>
#include <iosfwd>
#include <sys/types.h>
#include <vector>
#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/ResponseState/ResponseState.hpp"

class ConnectionState {
    private:    
        int eventFd;
        int epollFd;
        enum ReadState {NO_REQUEST, READING_HEADERS, READING_BODY, DONE_READING};
        ReadState readState;
        ssize_t bytesRead;
        std::string requestBuffer;
        HttpRequest *request;
        bool isRequestReady;
        bool isRequestDone;

        enum WriteState {NO_RESPONSE, SENDING_RESPONSE};
        WriteState writeState;
        ResponseState *responseState;
        
        bool isDone;
        uint32_t lastEvent;


        void updateEpollEvents(void);
    public:
        ConnectionState(int clientFd, int epollFd);
        HttpRequest *getHttpRequest(void) const;
        int getEventFd(void) const;
        bool getIsDone(void) const;
        bool getIsRequestReady(void) const;
        bool getIsRequestDone(void) const;
        void handleReadable(std::vector<Server> &servers);
        void setResponseState(ResponseState *responseState);
        void handleWritable(void);
        ~ConnectionState();
};




#endif