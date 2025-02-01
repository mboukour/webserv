#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"




void HttpResponse::handleGetRequest(const HttpRequest& request) {
    std::string path = request.getRequestBlock()->getRoot();
    throw HttpErrorException(request.getVersion(), NOT_IMPLEMENTED, "Not Implemented", "no code yet ;)", request.getRequestBlock()->getErrorPageHtml(NOT_IMPLEMENTED));
    
}