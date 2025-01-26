#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../AHttp/AHttp.hpp"

#include "../../Server/Server.hpp"

enum ERRORCODES {
    BAD_REQUEST=400,
    METHOD_NOT_ALLOWED=405,
    PAYLOAD_TOO_LARGE=413,
    UNAUTHORIZED=401,
    FORBIDDEN=403,
    NOT_FOUND=404,
    INTERNAL_SERVER_ERROR=505,
    NOT_IMPLEMENTED=501,
    SERVICE_UNAVAILABLE=503
};

class HttpRequest;

// HttpResponse will check everything
class HttpResponse: public AHttp {
    private:
        int statusCode;
        std::string reasonPhrase;
    public:
        HttpResponse();
        HttpResponse(const HttpRequest &request, const Server &server);
        HttpResponse(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &body);
        std::string toString(void) const;
};

#endif