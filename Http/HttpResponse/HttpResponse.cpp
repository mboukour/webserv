#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>



HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpRequest& request) {
    this->version = request.getVersion();

    if (request.getMethod() == "DELETE")
        handleDeleteRequest(request, request.getRequestBlock()->getRoot());
    else {
        this->statusCode = 405;
        this->reasonPhrase = "Method Not Allowed";
        this->body = "<html><body><h1>Method Not Allowed</h1></body></html>";
        this->headers["Content-Type"] = "text/html";
        std::stringstream ss;
        ss << this->body.size();
        this->headers["Content-Length"] = ss.str();
    }
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