#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "../ABlock/ABlock.hpp"
// #include "../Server.hpp"


class Server;
class Location: public ABlock 
{
    private:
        std::string locationName;
        const Server *myServer;
        
    public:
        Location();
        Location(const Location &other);
        void startServer(void);
        void setLocationName(const std::string &locationName);
        void setMyServer(const Server *server);
        const Server *getMyServer(void) const;
        std::string getLocationName(void) const;
};





#endif