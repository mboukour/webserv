#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../AHttp/AHttp.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Server/Server.hpp"


// HttpResponse will check everything
class HttpResponse: public AHttp {
    private:
        int statusCode;
        std::string reasonPhrase;

        void handleMaxBodySizeExceeded(void);
    public:
        HttpResponse(const HttpRequest &request, const Server &server);
        std::string toString(void) const;
};

#endif