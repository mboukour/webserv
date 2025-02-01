#include "Location.hpp"
#include "../Server.hpp"

Location::Location(): ABlock(), locationName(""), myServer(NULL) {}

void Location::startServer(void)
{
    throw std::logic_error("startServer() should never be called from within a location");
}

Location::Location(const Location& other)
    : ABlock(other), locationName(other.locationName), myServer(other.myServer) {}

void Location::setLocationName(const std::string &locationName) {
    this->locationName = locationName;
}

std::string Location::getLocationName(void) const {
    return (this->locationName);
}


void Location::setMyServer(const Server *server) {
    this->myServer = server;
}


const Server *Location::getMyServer(void) const {
    return (this->myServer);
}