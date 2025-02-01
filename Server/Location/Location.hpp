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
        
    public:
        Location();
        Location(const Location &other);
        Location &operator=(const Location &other);
        void startServer(void);
        void setLocationName(const std::string &locationName);
        std::string getLocationName(void) const;
        ~Location();
};





#endif