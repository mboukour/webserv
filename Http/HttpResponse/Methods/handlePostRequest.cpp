#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"




void HttpResponse::handlePostRequest(const HttpRequest& request) {
    std::string path = request.getRequestBlock()->getRoot();
    throw HttpErrorException(NOT_IMPLEMENTED, request, "Not Implemented");
}