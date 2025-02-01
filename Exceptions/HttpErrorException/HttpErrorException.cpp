#include "HttpErrorException.hpp"

HttpErrorException::HttpErrorException(int statusCode, const std::string &reasonPhrase, const std::string &message) : message(message), statusCode(statusCode), reasonPhrase(reasonPhrase) {}

std::string HttpErrorException::getErrorPageHtml(void) const {
    std::string html = "<!DOCTYPE html>\n<html>\n<head>\n<title>Error</p>\n</body>\n</html>";
    return html;
}

const char* HttpErrorException::what() const throw() {
    return this->message.c_str();
}

int HttpErrorException::getStatusCode(void) const {
    return this->statusCode;
}

HttpErrorException::~HttpErrorException() throw() {}
