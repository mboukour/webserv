#ifndef CGISTATE_HPP
#define CGISTATE_HPP

#include "../../ClientState/ClientState.hpp"
#include <cstddef>
#include <ctime>
#include <sys/types.h>
#include <vector>

class CgiState {
    private:
        static const int cgiTimeout;
        const int cgiFd;
        const int epollFd;
        const pid_t cgiPid;
        ClientState *client;
        time_t lastActivityTime;
        enum ReadMode {RAW_CHUNKED, CONTENT_LENGTH, READY_CHUNKED};
        ReadMode readMode;
        enum ReadState {READING_HEADERS, READING_BODY};
        ReadState readState;
        ssize_t contentSent;
        ssize_t contentLength;
        std::string currentChunk;
        std::string initBuffer;

        enum WriteState {NOT_REGISTERED, REGISTERED};
        WriteState writeState;
        std::vector<std::string> writeQueue;
        bool isResponding;
        bool isClean;
        bool isDone;

        CgiState();

        void updateLastActivity(void);
        void setupReadMode(size_t headersPos);
        void setupReadMode(const std::string &bufferStr);
        void readCgi(std::string bufferStr);
        void parseCgiHeaders(void);
        void sendCurrentChunk(void); // Dont forget to call this at the end marking the end chunk (if script exits??)
    public:
        CgiState(int cgiFd, int cgiPid, int epollFd, ClientState *clientToSend);
        void handlecgiReadable(void);
        void handleCgiWritable(void);
        void activateWriteState(const std::string &toWrite);
        void notifyCgiClient(int statusCode);
        bool isWritingDone(void) const;
        bool hasTimedOut(void) const;
        void cleanUpCgi(void);
        bool getIsDone(void) const;
        ClientState *getClient(void) const;
        bool isCgiAlive(void) const;
        ~CgiState();
};

#endif