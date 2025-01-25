#include "HttpResponse.hpp"
#include <sstream>


HttpResponse::HttpResponse(const HttpRequest& request, const Server &server) {
    this->version = request.getVersion();

    if (server.getIsLimited() && request.getBodySize() > server.getMaxBodySize())
    {
        handleMaxBodySizeExceeded();
        return ;
    }
    this->statusCode = 200;
    this->reasonPhrase = "OK";
    this->body = "<html><body><h1>Success</h1></body></html>";
    this->headers["Content-Type"] = "text/html";
    std::stringstream ss;
    ss << this->body.size();
    this->headers["Content-Length"] = ss.str();
}



void HttpResponse::handleMaxBodySizeExceeded(void) {
    this->statusCode = 413;
    this->reasonPhrase = "Payload Too Large";
    this->body = "<html><body><h1>Payload Too Large</h1></body></html>";
    this->headers["Content-Type"] = "text/html";
    std::stringstream ss;
    ss << this->body.size();
    this->headers["Content-Length"] = ss.str();
    // this body -> shou
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