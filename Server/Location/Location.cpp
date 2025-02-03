#include "Location.hpp"
#include "../Server.hpp"
#include <sstream>

Location::Location(): ABlock(), locationName("") {}

void Location::startServer(void)
{
    throw std::logic_error("startServer() should never be called from within a location");
}

Location::Location(const Location& other)
    : ABlock(other), locationName(other.locationName) {}

Location& Location::operator=(const Location& other)
{
    if (this == &other)
        return *this;
    ABlock::operator=(other);
    this->locationName = other.locationName;
    return *this;
}

void Location::setLocationName(const std::string &locationName) {
    this->locationName = locationName;
}


std::string Location::getLocationName(void) const {
    return (this->locationName);
}

void Location::setReturnDirective(const std::string &returnCode, const std::string &path) {
    std::stringstream ss;
    ss << returnCode;
    int code;
    ss >> code;
    this->returnDir.code = code;
    this->returnDir.path = path;
}

Location::~Location() {}