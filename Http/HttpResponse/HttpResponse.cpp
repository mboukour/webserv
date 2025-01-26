#include "HttpResponse.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../Exceptions/PayloadTooLargeException/PayloadTooLargeException.hpp"
#include "../../Exceptions/MethodNotAllowedException/MethodNotAllowedException.hpp"
#include "../../Exceptions/UnknownMethodException/UnknownMethodException.hpp"
#include "../../Exceptions/NotImplementedException/NotImplementedException.hpp"
#include "../../Exceptions/NotFoundException/NotFoundException.hpp"
#include "../../Exceptions/ConflictException/ConflictException.hpp"
#include "../../Exceptions/ForbiddenException/ForbiddenException.hpp"

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>

bool HttpResponse::removeDirectory(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return false;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..")
            continue;
        std::string fullPath = path + "/" + entryName;
        struct stat filestat;
        if (stat(fullPath.c_str(), &filestat) == 0)
        {
            if (S_ISDIR(filestat.st_mode))
            {
                if (!removeDirectory(fullPath))
                {
                    closedir(dir);
                    return false;
                }
            }
            else
            {
                if (remove(fullPath.c_str()) != 0)
                {
                    closedir(dir);
                    return false;
                }
            }
        }
    }
    closedir(dir);
    if (rmdir(path.c_str()) != 0)
        return false;
    return true;
}

void HttpResponse::handleDeleteRequest(const HttpRequest &request, const Server &server)
{
    std::string path = server.getRoot() + request.getPath();
    struct stat filestat;
    /*
    ** here i check if the file exists
    ** if it doesn't i return 404 (Not Found)
    */
    if (stat(path.c_str(), &filestat) != 0)
        throw NotFoundException();
    /*
    ** here i check if the file is a directory
    ** if it is i return 409 (Conflict)
    */
    if (S_ISDIR(filestat.st_mode))
    {
        if (request.getPath()[request.getPath().size() - 1] != '/')
            throw ConflictException();
        // Attempt to delete the directory
        if (!removeDirectory(path))
            throw ForbiddenException();
    } 
    else
    {
        // Attempt to delete a file
        if (remove(path.c_str()) != 0)
            throw ForbiddenException();
    }
    /*
    ** if everything went well i return 204 (No Content)
    */
    this->statusCode = 204;
    this->reasonPhrase = "No Content";
    std::cout << "Deleted: " << path << std::endl;
}

HttpResponse::HttpResponse() {}

HttpResponse::HttpResponse(const HttpRequest& request, const Server &server) {
    this->version = request.getVersion();

    if (request.getMethod() == "DELETE")
        handleDeleteRequest(request, server);
    else {
        this->statusCode = 405;
        this->reasonPhrase = "Method Not Allowed";
        this->body = "<html><body><h1>Method Not Allowed</h1></body></html>";
        this->headers["Content-Type"] = "text/html";
        std::stringstream ss;
        ss << this->body.size();
        this->headers["Content-Length"] = ss.str();
    }
}

HttpResponse::HttpResponse(const std::string &version, int statusCode,
    const std::string &reasonPhrase, const std::string &body) {
    this->version = version;
    this->statusCode = statusCode;
    this->reasonPhrase = reasonPhrase;
    this->body = body;
}



std::string HttpResponse::toString(void) const {
    std::stringstream ss;
    ss << this->version << " " << this->statusCode << " " << this->reasonPhrase << "\r\n";
    for(std::map<std::string, std::string >::const_iterator it = this->headers.begin(); 
        it != this->headers.end(); it++) {
            ss << it->first << ": " << it->second << "\r\n";
        }
    ss << "\r\n" << this->body;
    return (ss.str());
}