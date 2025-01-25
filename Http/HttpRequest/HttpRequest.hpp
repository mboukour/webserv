#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>

#include "../AHttp/AHttp.hpp"
#include "../HttpResponse/HttpResponseErrorMaker/HttpResponseErrorMaker.hpp"
#include "../../Server/Server.hpp"


class HttpRequest: public AHttp {
    private:
        std::string method;
        std::string path;
        std::string primalRequest;
    public:
        HttpRequest(const std::string &request, const Server &server); // throws exceptions that should never terminate execution of the program
        std::string getMethod() const;
        std::string getPath() const;
        std::string toString() const;
};







#endif