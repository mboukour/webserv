#include "BadRequestException.hpp"

BadRequestException::BadRequestException() : message("Bad request") {}

BadRequestException::BadRequestException(const std::string &reason) : message("Bad request: " + reason) {}

const char *BadRequestException::what() const throw() {
    return message.c_str();
}

BadRequestException::~BadRequestException() throw() {}

