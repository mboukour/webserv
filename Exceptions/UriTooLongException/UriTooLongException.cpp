#include "UriTooLongException.hpp"

UriTooLongException::UriTooLongException()
    : message("The URI is too long.") {}


const char* UriTooLongException::what() const throw() {
    return message.c_str();
}

UriTooLongException::~UriTooLongException() throw() {}
