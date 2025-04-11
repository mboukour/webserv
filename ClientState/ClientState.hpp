#ifndef ClientState_HPP
#define ClientState_HPP

#include <cstddef>
#include <ctime>
#include <iosfwd>
#include <sys/types.h>
#include <vector>
#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/HttpResponse.hpp"

#define READ_SIZE 65536
#define MAX_REQUEST 1000

class CgiState;
class ClientState {
    private:
        static const int keepAliveTimeout;
        const int eventFd;
        const int epollFd;
        enum ReadState {NO_REQUEST, READING_HEADERS, READING_BODY, DONE_READING};
        ReadState readState;
        ssize_t bytesRead;
        std::string requestBuffer;
        HttpRequest request;
        size_t requestCount;
        enum WriteState {NOT_REGISTERED, REGISTERED};
        WriteState writeState;
        enum SendMode {NONE, STRING, FILE};
        class SendMe {
            private:
                SendMe();
            public:
                SendMode sendMode;
                std::string filePath;
                std::streampos currentPos;
                std::string stringToSend;

                SendMe(const std::string &filePath, const std::streampos &currentPos);
                SendMe(const std::string &stringToSend);
                void changeSend(const std::string &filePath, const std::streampos &currentPos);
                void changeSend(const std::string &stringToSend);
        };
        std::vector<SendMe> sendQueue;
        HttpResponse *response;
        CgiState *cgiState;
        time_t lastActivityTime;
        bool isKeepAlive;
        bool isResponding;
        bool isDone;
        bool isClean;

        void resetReadState(void);
        void cleanUpEpoll(void);
        void updateLastActivity(void);
    public:
        ClientState(int clientFd, int epollFd);
        HttpResponse *getHttpResponse(void) const;
        const HttpRequest &getHttpRequest(void) const;
        int getEventFd(void) const;
        void setAsDone(void);
        bool getIsDone(void) const;
        void handleReadable(std::vector<Server> &servers);
        void activateWriteState(const std::string &filePath, const std::streampos &currentPos);
        void activateWriteState(const std::string &stringToSend);
        void handleWritable(void);
        bool hasTimedOut(void) const;
        bool getIsResponding(void) const;
        bool isSendingDone(void) const;
        bool getIsKeepAlive(void) const;
        ~ClientState();
};




#endif
