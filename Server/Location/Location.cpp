#include "Location.hpp"
#include "../Server.hpp"
#include <sstream>
#include <string>

Location::Location(): ABlock(), locationName(""), isReturnLocation(false) {
    this->returnDir.returnType = RETURN_URL;
    this->returnDir.code = -1;
    this->returnDir.path = "";
}

void Location::startServer(void)
{
    throw std::logic_error("startServer() should never be called from within a location");
}

Location::Location(const Location& other)
    : ABlock(other), locationName(other.locationName), isReturnLocation(other.isReturnLocation), returnDir(other.returnDir) {}

Location& Location::operator=(const Location& other)
{
    if (this == &other)
        return *this;
    ABlock::operator=(other);
    this->locationName = other.locationName;
    this->isReturnLocation = other.isReturnLocation;
    this->returnDir = other.returnDir;
    return *this;
}

void Location::setLocationName(const std::string &locationName) {
    this->locationName = locationName;
}


std::string Location::getLocationName(void) const {
    return (this->locationName);
}

void Location::setReturnDirective(const std::string &returnCode, const std::string &path, ReturnType type) {
    std::stringstream ss;
    ss << returnCode;
    int code;
    ss >> code;
    this->returnDir.code = code;
    this->returnDir.returnType = type;
    this->returnDir.path = path;
    this->isReturnLocation = true;
}

bool Location::getIsReturnLocation(void) const {
    return this->isReturnLocation;
}

ReturnType Location::getReturnType(void) const {
    return this->returnDir.returnType;
}

int Location::getReturnCode(void) const {
    return this->returnDir.code;
}

std::string Location::getReturnPath(void) const {
    return this->returnDir.path;
}


Location::~Location() {}