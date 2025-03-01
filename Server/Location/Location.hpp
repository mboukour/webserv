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
        bool isReturnLocation;
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
        bool getIsReturnLocation(void) const;
        int getReturnCode(void) const;
        std::string getReturnPath(void) const;
        std::string getLocationName(void) const;
        ~Location();
};





#endif