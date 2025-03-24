#ifndef CGISTATE_HPP
#define CGISTATE_HPP

#include "../../ClientState/ClientState.hpp"
#include <cstddef>
#include <ctime>
#include <sys/types.h>

class CgiState {
    private:
        int cgiRead;
        ClientState *client;
        pid_t cgiPid;
        enum ReadMode {RAW_CHUNKED, CONTENT_LENGTH, READY_CHUNKED};
        ReadMode readMode;
        enum ReadState {READING_HEADERS, READING_BODY};
        ReadState readState;

        
        ssize_t contentSent;

        ssize_t contentLength;
        
        std::string currentChunk;



        std::string initBuffer;
        bool isClean;
        bool isDone;
        CgiState();

        void setupReadMode(size_t headersPos);
        void setupReadMode(const std::string &bufferStr);
        void readCgi(std::string bufferStr);
        void parseCgiHeaders(void);
        void sendCurrentChunk(void); // Dont forget to call this at the end marking the end chunk (if script exits??)
        void cleanUpCgi(void);
    public:
        CgiState(int cgiRead, int cgiPid , ClientState *clientToSend);
        void handleCgiReadable(void);
        bool getIsDone(void) const;
        ~CgiState();
};

#endif