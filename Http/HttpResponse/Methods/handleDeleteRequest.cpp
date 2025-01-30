#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <dirent.h>


#include "../../../Exceptions/NotFoundException/NotFoundException.hpp"
#include "../../../Exceptions/ConflictException/ConflictException.hpp"
#include "../../../Exceptions/ForbiddenException/ForbiddenException.hpp"


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

void HttpResponse::handleDeleteRequest(const HttpRequest &request, const std::string &root)
{
    std::string path = root;

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
    {
        std::cout << "File not found: " << path << std::endl;
        throw NotFoundException();
    }
    /*
    ** Here I check if the file is a directory
    ** If it is, I return 409 (Conflict)
    */
    if (S_ISDIR(filestat.st_mode))
    {
        // Check if the directory path ends with '/'
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
    ** If everything went well, I return 204 (No Content)
    */
    this->statusCode = 204;
    this->reasonPhrase = "No Content";
    std::cout << "Deleted: " << path << std::endl;
}
