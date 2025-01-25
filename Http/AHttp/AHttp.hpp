#ifndef AHTTP_HPP
#define AHTTP_HPP

#include <string>
#include <map>

class AHttp { // This is an abstract class for stuff that both request and reposnse need
    protected:
        std::string version;
        size_t bodySize;
        std::string body;
        std::map<std::string, std::string> headers;

    public:
        std::string getVersion() const;
        size_t getBodySize() const;
        std::string getBody() const;
        std::string getHeader(const std::string &headerName) const;
        virtual std::string toString() const = 0;
        virtual ~AHttp() {}
};

#endif