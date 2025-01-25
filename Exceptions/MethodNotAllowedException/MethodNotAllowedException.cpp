#include "MethodNotAllowedException.hpp"

MethodNotAllowedException::MethodNotAllowedException() : message("Method Not Allowed") {}

MethodNotAllowedException::MethodNotAllowedException(const std::string &reason) : message("Method Not Allowed: " + reason) {}

const char *MethodNotAllowedException::what() const throw() {
    return message.c_str();
}

MethodNotAllowedException::~MethodNotAllowedException() throw() {}