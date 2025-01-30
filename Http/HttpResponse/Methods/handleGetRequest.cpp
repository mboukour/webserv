#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/PayloadTooLargeException/PayloadTooLargeException.hpp"
#include "../../../Exceptions/MethodNotAllowedException/MethodNotAllowedException.hpp"
#include "../../../Exceptions/UnknownMethodException/UnknownMethodException.hpp"
#include "../../../Exceptions/NotImplementedException/NotImplementedException.hpp"
#include "../../../Exceptions/NotFoundException/NotFoundException.hpp"
#include "../../../Exceptions/ConflictException/ConflictException.hpp"
#include "../../../Exceptions/ForbiddenException/ForbiddenException.hpp"

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <cstdio>
#include <dirent.h>

bool    HttpResponse::handleGetFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        throw ForbiddenException();
    file.seekg(0, std::ios::beg);
    const int  bufferSize = 50000;
    char buffer[bufferSize];
    this->body.clear();
    while (file.read(buffer, bufferSize))
        this->body.append(buffer, bufferSize);
    std::streamsize bytesRead = file.gcount();
    if (bytesRead > 0)
        this->body.append(buffer, bytesRead);
    std::stringstream ss;
    ss << this->body.size();
    this->headers["Content-Length"] = ss.str();
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension == "txt")
        this->headers["Content-Type"] = "text/plain";
    file.close();
    return (1);
}

bool    HttpResponse::handleGetDirectory(const std::string &path)
{
    std::string index = path + "/index.html";
    struct stat filestat;
    if (stat(index.c_str(), &filestat) == 0 && S_ISREG(filestat.st_mode))
        return (handleGetFile(index));
    DIR* dir = opendir(path.c_str());
    if (!dir)
        throw ForbiddenException();
    return (1);
}

int    HttpResponse::handleGetRequest(const HttpRequest &request,  const std::string &root)
{
    std::string path = root;

    if (request.getPath()[0] == '/')
        path += request.getPath().substr(1);
    else
        path += request.getPath();
    std::cout << "The path: " << path << std::endl;
    struct stat filestat;
    if (stat(path.c_str(), &filestat) == 0)
    {
        if (S_ISDIR(filestat.st_mode))
            this->isDirectory = true;
        else if (S_ISREG(filestat.st_mode))
            this->isDirectory = false;
    }
    else
        throw NotFoundException();
    this->version = request.getVersion();
    this->statusCode = 200;
    this->reasonPhrase = "OK";
    if (this->isDirectory)
    {
        if (handleGetDirectory(path))
            return (1);
    }
    else
    {
        if (handleGetFile(path))
            return (1); 
    }
    return (0);
}