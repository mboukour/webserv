#include "AHttp.hpp"


AHttp::AHttp(): version(""), bodySize(0) , body("") {}

std::string AHttp::getVersion() const {
    return this->version;
}

size_t AHttp::getBodySize() const {
    return this->bodySize;
}

std::string AHttp::getBody() const {
    return this->body;
}

std::string AHttp::getHeader(const std::string &headerName) const {
    std::map<std::string, std::string>::const_iterator it = this->headers.find(headerName);
    if (it != this->headers.end()) {
        return it->second;
    }
    return ""; // throw MissingHeader(headerName);
}