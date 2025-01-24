#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "../ABlock/ABlock.hpp"

class Location: public ABlock 
{
    private:
        std::string locationName;
    public:
        void startServer(void);
        void setLocationName(const std::string &locationName);
        std::string getLocationName(void) const;
};





#endif