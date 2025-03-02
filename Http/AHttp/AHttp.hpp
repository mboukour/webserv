#ifndef AHTTP_HPP
#define AHTTP_HPP

#include <string>
#include <map>


enum ERRORCODES {
    BAD_REQUEST=400,
    METHOD_NOT_ALLOWED=405,
    PAYLOAD_TOO_LARGE=413,
    UNAUTHORIZED=401,
    FORBIDDEN=403,
    NOT_FOUND=404,
    CONFLICT=409,
    URI_TOO_LONG=414,
    INTERNAL_SERVER_ERROR=500,
    NOT_IMPLEMENTED=501,
    SERVICE_UNAVAILABLE=503,
    GATEWAY_TIMEOUT=504,
    HTTP_VERSION_NOT_SUPPORTED=505
};


class AHttp { // This is an abstract class for stuff that both request and reposnse need
    protected:
        std::string version;
        size_t bodySize;
        std::string body;
        std::map<std::string, std::string> headers;
        

    public:
        AHttp();
        std::string getVersion() const;
        size_t getBodySize() const;
        std::string getBody() const;
        std::string getHeader(const std::string &headerName) const;
        void setHeader(const std::string &headerName, const std::string &headerValue);
        virtual std::string toString() const = 0;
        virtual ~AHttp() {}
};

#endif