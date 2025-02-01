#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"

#include "../../../Exceptions/HttpErrorException/ForbiddenException.hpp"

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <cstdio>
#include <dirent.h>


void    HttpResponse::handlePostRequest(const HttpRequest &request)
{
    std::string path = request.getRequestBlock()->getRoot();
}