#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../../Debug/Debug.hpp"
#include "../Server.hpp"
#include <vector>
#include <map>

enum State {
    PARSING_HEADERS,
    PARSING_BODY,
};

class ClientState {
    private:
    public:
        ClientState();
        int state;
        bool toIgnore;
        std::string receivedBuffer;
        HttpRequest request;

        void reset();
};

class ServerManager {
    private:
        int epollFd;
        std::vector<Server> &servers;
        std::map<int, ClientState> clientStates;  // Changed from vector to map
        void handleConnections(void);
        void removeClient(int clientFd);
        void sendResponse(const HttpRequest &request, int clientFd);
        void handleClientRead(int clientFd, char *buffer);
        void handleClient(int clientFd);
        void acceptConnections(int fdSocket);
        bool isAServerFdSocket(int fdSocket) const;
        const Server &getServer(int serverFd);

    public:
        ServerManager(std::vector<Server> &servers);
        void startServerManager(void);

};

#endif