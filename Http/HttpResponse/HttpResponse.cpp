#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>



HttpResponse::HttpResponse(): clientFd(-1) {}

HttpResponse::HttpResponse(const HttpRequest& request, int clientFd): clientFd(clientFd) {
    this->version = request.getVersion();

    const std::string &method = request.getMethod();
    if (request.isCgiRequest()) {
        std::string response = Cgi::getCgiResponse(request);
        response.insert(0, "HTTP/1.1 200 OK\r\n");
        // std::cout << "Received cgi's response: \n" << response << '\n';
        send(clientFd, response.c_str(), response.size(), 0);
        return ;
    }
    if (method == "DELETE")
        handleDeleteRequest(request);
    else if (method == "GET")
        handleGetRequest(request);
    else if (method == "POST")
        handlePostRequest(request);
}

HttpResponse::HttpResponse(const std::string &version, int statusCode,
    const std::string &reasonPhrase, const std::string &body): clientFd(-1) { // no client fd needed in case of error
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

void HttpResponse::sendResponse(void) const {
    std::string responseStr = this->toString();
    send(this->clientFd, responseStr.c_str(), responseStr.size(), 0);
}
