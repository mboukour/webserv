#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
// #include <unordered_map>
#include <vector>
#include "ABlock/ABlock.hpp"
#include "Location/Location.hpp"

class Server: public ABlock {
    private:
        int port;
        std::string serverName;
        // std::vector<Location> locations;
    public:
        int getPort(void) const;
        std::string getServerName(void) const;
        void setPort(int port);
        void setServerName(const std::string &serverName);
        void startServer(void);
};




#endif