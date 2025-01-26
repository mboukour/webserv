#ifndef HTTPRESPONSEMAKER_HPP
#define HTTPRESPONSEMAKER_HPP

#include "../HttpResponse.hpp"


class HttpResponseErrorMaker {
    private:
        static HttpResponse handleMaxBodySizeExceeded(void);
        static HttpResponse handleBadRequest(void);
        static HttpResponse handleMethodNotAllowed(void);
        static HttpResponse handleUnauthorized(void);
        static HttpResponse handleForbidden(void);
        static HttpResponse handleNotFound(void);
        static HttpResponse handleInternalServerError(void);
        static HttpResponse handleNotImplemented(void);
        static HttpResponse handleServiceUnavailable(void);

    public:
        static HttpResponse makeHttpResponseError(int errorCode);
};







#endif