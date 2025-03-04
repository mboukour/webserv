#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <map>
#include "../Server.hpp"

#define MAX_EVENTS 10

class HttpRequest;
class ConnectionState;

class ServerManager {
    private:
        static int epollFd;
        static std::vector<Server> servers;
        static std::map<int, ConnectionState*> clientStates;
        
        // Private constructor to prevent instantiation
        ServerManager();

        static bool isAServerFdSocket(int fdSocket);
        static const Server &getServer(int port);
        static void handleConnections(void);
        static void acceptConnections(int fdSocket);
        static void sendResponse(HttpRequest &request, int clientFd);
        
        public:
        // Initialize with server list
        static void initialize(std::vector<Server> &serversList);
        // Start the server manager (previously startServerManager)
        static void start(void);
        static ConnectionState *getConnectionState(int clientFd);
};

#endif // SERVERMANAGER_HPP