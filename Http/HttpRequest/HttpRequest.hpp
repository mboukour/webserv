#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <vector>

#include "../AHttp/AHttp.hpp"
#include "../../Server/Server.hpp"


#define URI_MAX_SIZE 2048

class HttpRequest: public AHttp {
    private:
        std::string method;
        std::string path;
        std::string queryString;
        std::string primalRequest;
        bool isCgi;
        const Server *server;
        const ABlock *requestBlock;

        void setIsCgi(void) ;
        static const Server& getServer(const std::string &host, const std::vector<Server> &servers, int serverPort);
    public:
        HttpRequest(const std::string &request, const std::vector<Server>& servers, int serverPort); // throws exceptions that should never terminate execution of the program
        std::string getMethod() const;
        std::string getPath() const;
        std::string getQueryString() const;
        std::string toString() const;
        bool isCgiRequest(void) const;
        const Server *getServer(void) const;
        const ABlock *getRequestBlock(void) const;
        void printHeaders(void) const;
};







#endif