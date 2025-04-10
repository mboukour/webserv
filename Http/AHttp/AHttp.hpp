#ifndef AHTTP_HPP
#define AHTTP_HPP

#include <string>
#include <map>





class AHttp {
    protected:
        std::string version;
        size_t bodySize;
        std::string body;
        std::map<std::string, std::string> headers;
        static std::string uriAllowedChars;
        static void encoding(std::string &replace);

    public:
        AHttp();
        static std::string sanitizePath(std::string path);
        std::string getVersion() const;
        size_t getBodySize() const;
        std::string getBody() const;
        std::string getHeader(const std::string &headerName) const;
        void setHeader(const std::string &headerName, const std::string &headerValue);
        virtual std::string toString() const = 0;
        virtual ~AHttp() {}
};

#endif