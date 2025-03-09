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
        enum PostState {INIT_POST, NEW_REQ_ENTRY, LAST_ENTRY};
        PostState postState;
        int fd;
        std::string reasonPhrase;
        std::vector<std::string> cookies;

        static std::string getConTypeExten(const std::string &contentType);
        static std::string extToNature(const std::string &extension);

        void sendGetResponse(std::fstream &fileToGet, const std::string &filePath) const;
        void handleAutoIndex(const HttpRequest& request) const;

        void sendResponse(void) const;
        void handleDeleteRequest(const HttpRequest &request);
        void handleGetRequest(const HttpRequest &request);
        void handlePostRequest(const HttpRequest &request);

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpRequest &request, int clientFd, int epollFd);
        HttpResponse(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &body);
        void addCookie(const std::string& name, const std::string& value, const std::string& attributes);
        void setBody(const std::string &body);
        bool removeDirectory(const std::string &path);
        void handleNewReqEntry(const HttpRequest &request);
        void setAsLastEntry(void);
        std::string toString(void) const;
};

#endif
