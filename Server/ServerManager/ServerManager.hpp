#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../../Debug/Debug.hpp"
#include "../Server.hpp"

class ServerManager {
    private:
        int epollFd;
        std::vector<Server> &servers;
        void handleConnections(void);
        void handleClient(int clientFd);
        void acceptConnections(int fdSocket);
        bool isAServerFdSocket(int fdSocket) const;
        const Server &getServer(int serverFd);
        // ServerManager();
    public:
        ServerManager(std::vector<Server> &servers);
        void startServerManager(void);

};


#endif