#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
class HttpRequest {
    private:
        std::string method;
        std::string path;
        std::string version;
        size_t bodySize;
        std::string body;

    public:
        std::map<std::string, std::string> headers;
        HttpRequest(const std::string &request); // throws exceptions that should never terminate execution of the program
        std::string getMethod() const;
        std::string getPath() const;
        std::string getVersion() const;
        size_t getBodySize() const;
        std::string getBody() const;
        std::string getHeader(const std::string &headerName) const;
        // std::string respond(void) const;
};







#endif