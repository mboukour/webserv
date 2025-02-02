#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include "../ABlock/ABlock.hpp"
// #include "../Server.hpp"


class Server;
class Location: public ABlock 
{
    private:
        std::string locationName;
        struct returnDirective {
            int code;
            std::string path;
        } returnDir;

    public:
        Location();
        Location(const Location &other);
        Location &operator=(const Location &other);
        void startServer(void);
        void setLocationName(const std::string &locationName);
        void setReturnDirective(const std::string &returnCode, const std::string &path);
        std::string getLocationName(void) const;
        ~Location();
};





#endif