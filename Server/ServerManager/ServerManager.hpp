#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <sys/epoll.h>
#include <vector>
#include <map>
#include "../Server.hpp"
#include "../../EpollEvent/EpollEvent.hpp"

#define MAX_EVENTS 10

class HttpRequest;
class ClientState;

class ServerManager {
    private:
        static int epollFd;
        static std::vector<Server> servers;
        static std::map<int, EpollEvent*> eventStates;
        

        ServerManager();

        static bool isAServerFdSocket(int fdSocket);
        static const Server &getServer(int port);
        static void handleConnections(void);
        static void acceptConnections(int fdSocket);
        static void clientServerEpoll(EpollEvent *epollEvent, struct epoll_event &event);
        static void cgiEpoll(EpollEvent *epollEvent, struct epoll_event &event);
    public:
        // static void sendResponse(HttpRequest &request, int clientFd);
        static void sendString(const std::string &str, int clientFd);
        static void sendFile(const std::string &filePath, int clientFd);
        static ClientState *getClientState(int clientFd);
        static EpollEvent *getEpollEvent(int clientFd);
        // void registerEvent(void);

        static void initialize(std::vector<Server> &serversList);
        static void start(void);
        static void cleanUp(void);
};

#endif