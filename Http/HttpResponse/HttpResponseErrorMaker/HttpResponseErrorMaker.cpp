#include "HttpResponseErrorMaker.hpp"

HttpResponse HttpResponseErrorMaker::handleMaxBodySizeExceeded() {
    return HttpResponse("HTTP/1.1", 413, "Payload Too Large", "<html><body><h1>413 Payload Too Large</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleBadRequest() {
    return HttpResponse("HTTP/1.1", 400, "Bad Request", "<html><body><h1>400 Bad Request</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleMethodNotAllowed() {
    return HttpResponse("HTTP/1.1", 405, "Method Not Allowed", "<html><body><h1>405 Method Not Allowed</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleUnauthorized() {
    return HttpResponse("HTTP/1.1", 401, "Unauthorized", "<html><body><h1>401 Unauthorized</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleForbidden() {
    return HttpResponse("HTTP/1.1", 403, "Forbidden", "<html><body><h1>403 Forbidden</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleNotFound() {
    return HttpResponse("HTTP/1.1", 404, "Not Found", "<html><body><h1>404 Not Found</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleInternalServerError() {
    return HttpResponse("HTTP/1.1", 500, "Internal Server Error", "<html><body><h1>500 Internal Server Error</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleNotImplemented() {
    return HttpResponse("HTTP/1.1", 501, "Not Implemented", "<html><body><h1>501 Not Implemented</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::handleServiceUnavailable() {
    return HttpResponse("HTTP/1.1", 503, "Service Unavailable", "<html><body><h1>503 Service Unavailable</h1></body></html>");
}

HttpResponse HttpResponseErrorMaker::makeHttpResponseError(int errorCode) {
    switch (errorCode) {
        case 400:
            return handleBadRequest();
        case 401:
            return handleUnauthorized();
        case 403:
            return handleForbidden();
        case 404:
            return handleNotFound();
        case 405:
            return handleMethodNotAllowed();
        case 413:
            return handleMaxBodySizeExceeded();
        case 500:
            return handleInternalServerError();
        case 501:
            return handleNotImplemented();
        case 503:
            return handleServiceUnavailable();
        default:
            return handleInternalServerError();
    }
}