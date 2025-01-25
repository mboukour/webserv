#include "PayloadTooLargeException.hpp"
#include <sstream>
PayloadTooLargeException::PayloadTooLargeException() : message("Payload Too Large") {}

PayloadTooLargeException::PayloadTooLargeException(size_t maxBodySize) : message("Payload Too Large, maximun possible body size: ") {
    std::stringstream ss;
    ss << maxBodySize;
    message += ss.str();
}

const char *PayloadTooLargeException::what() const throw() {
    return message.c_str();
}

PayloadTooLargeException::~PayloadTooLargeException() throw() {}