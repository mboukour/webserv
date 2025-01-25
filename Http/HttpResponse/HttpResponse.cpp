#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Exceptions/PayloadTooLargeException/PayloadTooLargeException.hpp"
#include "../../Exceptions/MethodNotAllowedException/MethodNotAllowedException.hpp"
#include "../../Exceptions/UnknownMethodException/UnknownMethodException.hpp"
#include "../../Exceptions/NotImplementedException/NotImplementedException.hpp"

#include <sstream>

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpRequest& request, const Server &server) {
    this->version = request.getVersion();

    (void)server; // REMOVE IF WE DONT NEED IT LATER
    this->statusCode = 200;
    this->reasonPhrase = "OK";
    this->body = "<html><body><h1>Success</h1></body></html>";
    this->headers["Content-Type"] = "text/html";
    std::stringstream ss;
    ss << this->body.size();
    this->headers["Content-Length"] = ss.str();
}

HttpResponse::HttpResponse(const std::string &version, int statusCode,
    const std::string &reasonPhrase, const std::string &body) {
    this->version = version;
    this->statusCode = statusCode;
    this->reasonPhrase = reasonPhrase;
    this->body = body;
}



std::string HttpResponse::toString(void) const {
    std::stringstream ss;
    ss << this->version << " " << this->statusCode << " " << this->reasonPhrase << "\r\n";
    for(std::map<std::string, std::string >::const_iterator it = this->headers.begin(); 
        it != this->headers.end(); it++) {
            ss << it->first << ": " << it->second << "\r\n";
        }
    ss << "\r\n" << this->body;
    return (ss.str());
}