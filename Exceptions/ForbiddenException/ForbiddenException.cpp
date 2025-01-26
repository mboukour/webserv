#include "ForbiddenException.hpp"

ForbiddenException::ForbiddenException() : message("Forbidden") {}

ForbiddenException::ForbiddenException(const std::string &reason) : message(reason) {}

const char *ForbiddenException::what() const throw() {
    return message.c_str();
}

ForbiddenException::~ForbiddenException() throw() {}

