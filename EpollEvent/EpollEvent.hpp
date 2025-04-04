#ifndef EPOLLEVENT_HPP
#define EPOLLEVENT_HPP


#include "../ClientState/ClientState.hpp"
#include "../Cgi/CgiState/CgiState.hpp"
#include <ctime>


class EpollEvent {
    public:
        enum EventType {
            SERVER_SOCKET,
            CLIENT_CONNECTION, 
            CGI_READ,
        };
        EpollEvent(int eventFd, int epollFd, EventType eventType);
        EpollEvent(int cgiFd, int cgiPid, int epollFd , ClientState *clientToSend);
        EventType getEventType(void) const;
        ClientState *getClientState(void) const;
        CgiState *getCgiState(void) const;
        int getEventFd(void) const;
        bool getIsDone(void) const;
        bool hasTimedOut(void) const;
        ~EpollEvent();
    private:
        const int eventFd;
        const int epollFd;
        const EventType eventType;
        bool isClean;
        union EventData {
            ClientState *clientState;
            CgiState *cgiState;
        } eventData;

        void cleanUpEpollEvent(void);
};


#endif