#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <sstream>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>
#include "../../Server/ServerManager/ServerManager.hpp"


HttpResponse::HttpResponse(): clientFd(-1), epollFd(-1), fd(-1){}



HttpResponse::HttpResponse(const HttpRequest& request, int clientFd, int epollFd): clientFd(clientFd), epollFd(epollFd), postState(INIT_POST), fd(-1){
    this->version = request.getVersion();

    const std::string &method = request.getMethod();
    if (request.isCgiRequest()) {
        std::string response = Cgi::getCgiResponse(request);
        response.insert(0, "HTTP/1.1 200 OK\r\n");
        // std::cout << "Received cgi's response: \n" << response << '\n';
        ServerManager::sendString(response, clientFd);
        return ;
    }
    if (method == "DELETE")
        handleDeleteRequest(request);
    else if (method == "GET")
        handleGetRequest(request);
    else if (method == "POST")
        handlePostRequest(request);
    if (request.getHeader("Connection") == "close")
    {
        epoll_ctl(this->epollFd, EPOLL_CTL_DEL, this->clientFd, NULL);
        close(clientFd);
    }
}

void HttpResponse::handleNewReqEntry(const HttpRequest &request) {
    if (request.getMethod() != "POST")
        return ;
    handlePostRequest(request);
}

HttpResponse::HttpResponse(const std::string &version, int statusCode,
    const std::string &reasonPhrase, const std::string &body): clientFd(-1), epollFd(-1) { // no client fd needed in case of error
    this->version = version;
    this->statusCode = statusCode;
    this->reasonPhrase = reasonPhrase;
    this->body = body;
}

void HttpResponse::addCookie(const std::string& name, const std::string& value, const std::string& attributes) {
    std::string cookie = "Set-Cookie: " + name + "=" + value;
    if (!attributes.empty())
        cookie += "; " + attributes;
    cookies.push_back(cookie);
}

void HttpResponse::setBody(const std::string &body) {
    this->body = body;
    this->bodySize = body.size();
    std::stringstream ss;
    ss << this->bodySize;
    this->headers["Content-Length"] = ss.str();
}

std::string HttpResponse::toString(void) const {
    std::stringstream ss;
    ss << this->version << " " << this->statusCode << " " << this->reasonPhrase << "\r\n";
    for(std::map<std::string, std::string >::const_iterator it = this->headers.begin();
        it != this->headers.end(); it++) {
            ss << it->first << ": " << it->second << "\r\n";
        }
    for (std::vector<std::string>::const_iterator ite = cookies.begin(); ite != cookies.end(); ++ite)
        ss  << *ite << "\r\n";
    ss << "\r\n" << this->body;
    return (ss.str());
}

void HttpResponse::sendResponse(void) const {
    std::string responseStr = this->toString();
    ServerManager::sendString(responseStr, this->clientFd);
}

HttpResponse::~HttpResponse(){
    close(this->fd);
}
