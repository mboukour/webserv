#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>
#include "../ABlock/ABlock.hpp"
// #include "../Server.hpp"


class Server;

enum ReturnType {RETURN_URL, RETURN_BODY};
class Location: public ABlock 
{
    private:
        std::string locationName;
        bool isReturnLocation;
        struct returnDirective {
            ReturnType returnType;
            int code;
            std::string path;
        } returnDir;

    public:
        Location();
        Location(const Location &other);
        Location &operator=(const Location &other);
        void startServer(void);
        void setLocationName(const std::string &locationName);
        void setReturnDirective(const std::string &returnCode, const std::string &path, ReturnType type);
        bool getIsReturnLocation(void) const;
        int getReturnCode(void) const;
        std::string getReturnPath(void) const;
        ReturnType getReturnType(void) const;
        std::string getLocationName(void) const;
        ~Location();
};





#endif