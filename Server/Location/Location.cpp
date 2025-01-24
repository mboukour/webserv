#include "Location.hpp"


Location::Location(): ABlock(), locationName("") {}

void Location::startServer(void)
{
    throw std::logic_error("startServer() should never be called from within a location");
}

Location::Location(const Location& other)
    : ABlock(other), locationName(other.locationName) {}

void Location::setLocationName(const std::string &locationName) {
    this->locationName = locationName;
}

std::string Location::getLocationName(void) const {
    return (this->locationName);
}