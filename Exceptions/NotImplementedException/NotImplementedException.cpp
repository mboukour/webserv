#include "NotImplementedException.hpp"

NotImplementedException::NotImplementedException() : message("Not Implemented") {}

NotImplementedException::NotImplementedException(const std::string &reason) : message("Not Implemented: " + reason) {}

const char *NotImplementedException::what() const throw() {
    return message.c_str();
}

NotImplementedException::~NotImplementedException() throw() {}