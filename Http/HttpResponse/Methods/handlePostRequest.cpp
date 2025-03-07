#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <fstream>
#include "../../../Debug/Debug.hpp"
#include "../../../Utils/Logger/Logger.hpp"


void HttpResponse::handlePostRequest(const HttpRequest& request) {
    // (void)request;
    if (this->postState == INIT_POST) {
        this->postState = NEW_REQ_ENTRY; // DO NOT REMOVE THIS
        // here you should have all headers and stuff
        std::cout << "NEW REQ: " << request;
        //
    } else {
        // hna stuff you receive as the request progresses (little parts from body until its done)
        std::cout << "NEW ENTRY\n";
        
    }
    // throw HttpErrorException(NOT_IMPLEMENTED, request, "Not Implemented");
}