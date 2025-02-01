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
        std::string host;
        int fdSocket;
        std::string serverName;
        std::vector<Location> locations;

    public:
        Server();
        Server(const Server& other);
        std::string getServerName(void) const;
        void setPort(int port);
        int getPort(void) const;
        int getFdSocket(void) const;
        void setServerName(const std::string &serverName);
        void setHost(const std::string &host);
        std::string getHost(void) const;
        std::vector<Location>::iterator locationsBegin(void) ;
        std::vector<Location>::iterator locationsEnd(void) ;
        std::vector<Location>::const_iterator locationsCbegin(void) const;
        std::vector<Location>::const_iterator locationsCend(void) const;

        void addLocation(const Location& location);
        void startServer(void);
        ~Server();
};




#endif