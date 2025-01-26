#include "HttpRequestParseException.hpp"

HttpRequestParseException::HttpRequestParseException() : message("HTTP Request Parsing Error") {}

HttpRequestParseException::HttpRequestParseException(const std::string &reason) : message("HTTP Request Parsing Error: " + reason) {}

const char *HttpRequestParseException::what() const throw() {
    return message.c_str();
}

HttpRequestParseException::~HttpRequestParseException() throw() {}