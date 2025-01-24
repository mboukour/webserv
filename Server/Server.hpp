#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include "ABlock/ABlock.hpp"
#include "Location/Location.hpp"

class Server: public ABlock {
    private:
        int port;
        std::string serverName;
    public:
        std::vector<Location> locations;

        int getPort(void) const;
        std::string getServerName(void) const;
        void setPort(int port);
        void setServerName(const std::string &serverName);
        void startServer(void);
};




#endif