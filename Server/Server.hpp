#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <string>
#include <iostream>
#include "ABlock/ABlock.hpp"
#include "Location/Location.hpp"

#define MAX_EVENTS 10

class Server: public ABlock {
    private:
        int port;
        int fdSocket;
        std::string serverName;

    public:
        Server();
        Server(const Server& other);
        std::vector<Location> locations;
        std::string getServerName(void) const;
        void setPort(int port);
        int getPort(void) const;
        int getFdSocket(void) const;
        void setServerName(const std::string &serverName);
        void startServer(void);
        ~Server();
};




#endif