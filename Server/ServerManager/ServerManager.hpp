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
        

        ServerManager();

        static bool isAServerFdSocket(int fdSocket);
        static const Server &getServer(int port);
        static void handleConnections(void);
        static void acceptConnections(int fdSocket);
        
    public:
        // static void sendResponse(HttpRequest &request, int clientFd);
        static void sendString(const std::string &str, int clientFd);
        static void sendFile(const std::string &filePath, int clientFd);
        static ConnectionState *getConnectionState(int clientFd);

        static void initialize(std::vector<Server> &serversList);
        static void start(void);
        static void cleanUp(void);
};

#endif