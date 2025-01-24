#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include "ABlock/ABlock.hpp"
#include "Location/Location.hpp"

#define MAX_EVENTS 10

class Server: public ABlock {
    private:
        int port;
        int fdSocket;
        int epollFd;
        std::string serverName;
        void acceptConnections(void) const;
        void handleConnections(void) const;
        void handleClient(int clientFd) const;
    public:
        std::vector<Location> locations;
        std::string getServerName(void) const;
        void setPort(int port);
        int getPort(void) const;
        void setServerName(const std::string &serverName);



        void startServer(void);
};




#endif