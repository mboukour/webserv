#include "ConflictException.hpp"

ConflictException::ConflictException() : message("Conflict") {}

ConflictException::ConflictException(const std::string &reason) : message(reason) {}

const char *ConflictException::what() const throw() {
    return message.c_str();
}

ConflictException::~ConflictException() throw() {}

