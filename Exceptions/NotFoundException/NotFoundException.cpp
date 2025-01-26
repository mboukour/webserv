#include "NotFoundException.hpp"

NotFoundException::NotFoundException() : message("Not Found") {}

NotFoundException::NotFoundException(const std::string &reason) : message("Not Found: " + reason) {}

const char *NotFoundException::what() const throw() {
    return message.c_str();
}

NotFoundException::~NotFoundException() throw() {}