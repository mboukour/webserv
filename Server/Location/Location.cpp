#include "Location.hpp"


void Location::startServer(void)
{
    throw std::logic_error("startServer() should never be called from within a location");
}


void Location::setLocationName(const std::string &locationName) {
    this->locationName = locationName;
}

std::string Location::getLocationName(void) const {
    return (this->locationName);
}