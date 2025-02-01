#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"

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
    // if (rmdir(path.c_str()) != 0)
    //     return false;
    return true;
}

void HttpResponse::handleDeleteRequest(const HttpRequest &request)
{
    std::string path = request.getRequestBlock()->getRoot();

    // Manually check if the first character is '/'
    if (request.getPath()[0] == '/')
        path += request.getPath().substr(1);  // Remove the leading '/' if it exists
    else
        path += request.getPath();  // No leading '/' to remove
    std::cout << "Current path: "  << path << '\n';
    struct stat filestat;
    /*
    ** Here I check if the file exists
    ** If it doesn't, I return 404 (Not Found)
    */
    if (stat(path.c_str(), &filestat) != 0)
        throw HttpErrorException(request.getVersion(), NOT_FOUND, "Not Found", "file not found: " + path, request.getRequestBlock()->getErrorPageHtml(NOT_FOUND));
    /*
    ** Here I check if the file is a directory
    ** If it is, I return 409 (Conflict)
    */
    if (S_ISDIR(filestat.st_mode))
    {
        // Check if the directory path ends with '/'
        if (request.getPath()[request.getPath().size() - 1] != '/')
            throw HttpErrorException(request.getVersion() ,CONFLICT, "Conflict", "directory doesn't end with /", request.getRequestBlock()->getErrorPageHtml(CONFLICT));

        // Attempt to delete the directory
        if (!removeDirectory(path))
            throw HttpErrorException(request.getVersion() ,FORBIDDEN, "Forbidden", "removeDirectory() returned false", request.getRequestBlock()->getErrorPageHtml(FORBIDDEN));

    }
    else
    {
        // Attempt to delete a file
        if (remove(path.c_str()) != 0) // do we need to check errno for better error management???
            throw HttpErrorException(request.getVersion() ,FORBIDDEN, "Forbidden", "remove() failed", request.getRequestBlock()->getErrorPageHtml(FORBIDDEN));
            
    }

    /*
    ** If everything went well, I return 204 (No Content)
    */
    this->statusCode = 204;
    this->reasonPhrase = "No Content";
    std::cout << "Deleted: " << path << std::endl;
    this->sendResponse();
}
