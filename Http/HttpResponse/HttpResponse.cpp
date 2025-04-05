#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cstddef>
#include <sstream>
#include <fstream>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>
#include "../../Server/ServerManager/ServerManager.hpp"
#include "../../Session/Login/Login.hpp"

HttpResponse::HttpResponse(): clientFd(-1), epollFd(-1), fd(-1){}



HttpResponse::HttpResponse(const HttpRequest& request, int clientFd, int epollFd):
    clientFd(clientFd), epollFd(epollFd), postState(INIT_POST), prevPostState(INIT_POST), fd(-1), fileName(),
    chunkState(CH_START), remaining_chunk_size(0), offset(0), chunkBody(""), left(0), packet(""), prev_chunk_size(""), pendingCRLF(false),
    isLastEntry(false), multiState(M_BOUND), currBound(0), written(0) {
    if (handleSessionTest(request) || handleReturnDirective(request))
        return;
    this->version = request.getVersion();
    std::ifstream htmlFile("Http/HttpResponse/upload_pages/success_create.html"); // html page when the upload is successfull, ruined can store in a better place
    if (!htmlFile.is_open()) {
        throw HttpErrorException(500, request, "Failed to open HTML file");
    }
    std::stringstream sS;
    sS << htmlFile.rdbuf();
    this->success_create = sS.str();
    htmlFile.close() ;

    if (request.getMethod() == "POST" && request.getContentLength() == request.getBodySize()) {
        this->prevPostState = this->postState;
        this->postState = LAST_ENTRY;
    }
    const std::string &method = request.getMethod();
    if (request.isCgiRequest()) {
        this->cgiState = Cgi::initCgi(request, this->clientFd, this->epollFd);
        if (method == "POST")
            handlePostRequest(request);
        return ;
    }
    if (method == "DELETE")
        handleDeleteRequest(request);
    else if (method == "GET")
        handleGetRequest(request);
    else if (method == "POST")
        handlePostRequest(request);
    request.getRequestBlock()->getUploadPath();
}


bool HttpResponse::isReturnRequest(const HttpRequest &request) {
    const Location *loc = dynamic_cast< const Location *>(request.getRequestBlock());
    if (!loc || !loc->getIsReturnLocation())
        return false;
    return true;
}


bool HttpResponse::handleSessionTest(const HttpRequest &request) const {
    if (request.getPath() == "/session-test") {
        Login::respondToLogin(request, this->clientFd);
        return true;
    }
    return false;
}

bool HttpResponse::handleReturnDirective(const HttpRequest &request) const {
    const Location *loc = dynamic_cast< const Location *>(request.getRequestBlock());
    if (!loc || !loc->getIsReturnLocation())
        return false;
    HttpResponse resp(request.getVersion(), loc->getReturnCode(),
        HttpErrorException::getReasonPhrase(loc->getReturnCode()), "");
    switch (loc->getReturnType()) {
        case RETURN_URL:
            resp.setHeader("Location", loc->getReturnPath());
            break;
        case RETURN_BODY:
            resp.setBody(loc->getReturnPath());
            break;
    }
    ServerManager::sendString(resp.toString(), this->clientFd);
    std::cout << resp.toString() << std::endl;
    return true;
}

void HttpResponse::handleNewReqEntry(const HttpRequest &request) {
    if (request.getMethod() != "POST" || isReturnRequest(request))
        return ;
    handlePostRequest(request);
}

bool HttpResponse::getIsLastEntry(void) const {
    return this->isLastEntry;
}

HttpResponse::HttpResponse(const std::string &version, int statusCode,
    const std::string &reasonPhrase, const std::string &body): clientFd(-1), epollFd(-1), fd(-1) { // no client fd needed in case of error
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

void HttpResponse::addNeededHeaders(void) {
    if (this->headers.find("Server") == this->headers.end())
        this->headers["Server"] = "Webserv 1.0";
    if (this->headers.find("Date") == this->headers.end()) {
        time_t now = time(0);
        struct tm *gmt = gmtime(&now);
        char date_buffer[100];
        strftime(date_buffer, sizeof(date_buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
        this->headers["Date"] = date_buffer;
    }

}

bool HttpResponse::isCgiFile(const std::string &filePath, const HttpRequest &request) {
    std::string scriptName;
    std::string word;
    std::stringstream ss(filePath);

    while(getline(ss, word, '/')) {

        size_t pos = word.find_last_of('.');
        if (pos != std::string::npos)
        {
            std::string extension = word.substr(pos + 1);
            return  Cgi::isValidCgiExtension(extension, request);
        }
    }
    return false;
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
    if (this->fd != -1)
        close(this->fd);
}

int HttpResponse::getFd(void) const{
    return this->fd;
}
