#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../AHttp/AHttp.hpp"

#include "../../Server/Server.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../Cgi/Cgi.hpp"

class HttpRequest;

// HttpResponse will check everything
class HttpResponse: public AHttp {
    private:
        const int clientFd;
        const int epollFd;
        int statusCode;
        std::string reasonPhrase;

        void sendGetResponse(std::fstream &fileToGet, const std::string &filePath) const;
        void handleAutoIndex(const HttpRequest& request) const;

        void sendResponse(void) const;  
    public:
        HttpResponse();
        HttpResponse(const HttpRequest &request, int clientFd, int epollFd);
        HttpResponse(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &body);
        void handleDeleteRequest(const HttpRequest &request);
        void handleGetRequest(const HttpRequest &request);
        void handlePostRequest(const HttpRequest &request);
        bool removeDirectory(const std::string &path);
        std::string toString(void) const;
};

#endif