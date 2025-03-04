#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <fstream>




void HttpResponse::handlePostRequest(const HttpRequest& request) {
    std::string path = request.getRequestBlock()->getRoot();
    std::ofstream toWriteTo("hi.txt");
    toWriteTo << request.getBody();
    toWriteTo.close();
    throw HttpErrorException(NOT_IMPLEMENTED, request, "Not Implemented");
}