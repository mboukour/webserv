#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../../Debug/Debug.hpp"
#include <time.h>
#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cstddef>
#include <ostream>
#include <string>
#include <sys/time.h>
#include <sys/stat.h>
#include <sstream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include "../../../Debug/Debug.hpp"
#include "../../../ConnectionState/ConnectionState.hpp"
#include "../../../Server/ServerManager/ServerManager.hpp"

std::string regexReplace(const std::string& filename) {
    std::string result;
    for (size_t i = 0; i < filename.size(); i++) {
        if ((filename[i] >= 'a' && filename[i] <= 'z') || // lowercase letters
            (filename[i] >= 'A' && filename[i] <= 'Z') || // uppercase letters
            (filename[i] >= '0' && filename[i] <= '9') || // digits
            (filename[i] == '_') ||              // underscore
            (filename[i] == '.') ||              // period
            (filename[i] == '-')) {               // hyphen
            result += filename[i]; // Keep the character
        } else {
            result += '_'; // Replace with underscore
        }
    }
    return result;
}

std::string randomizeFileName(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long seed = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    srand(seed);
    std::stringstream sS;
    sS << rand();
    std::string fileID = sS.str();
    return fileID;
}

// if the transfer encoding is set tp chunked, the content-lenght header presence is not necessary
// map with the content-type and it corresponding file extension
// parse is chunked is set

// why sometimes i get "Fatal Runtime error: BODY ISZE BIGGER THAN CL"

// TODO:
// generate response: the randomized file name must be mentioned in the response headers
// create && fill the file

std::string HttpResponse::getConTypeExten(const std::string &contentType) {
    if (contentType == "text/html") return ".html";
    else if (contentType == "text/plain") return ".txt";
    else if (contentType == "image/jpeg") return ".jpg";
    else if (contentType == "image/png") return ".png";
    else if (contentType == "image/gif") return ".gif";
    else if (contentType == "application/json") return ".json";
    else if (contentType == "application/xml") return ".xml";
    else if (contentType == "application/pdf") return ".pdf";
    else if (contentType == "application/zip") return ".zip";
    else if (contentType == "audio/mpeg") return ".mp3";
    else if (contentType == "audio/mp3") return ".mp3";
    else if (contentType == "video/mp4") return ".mp4";
    return ""; // Return an empty string if no match is found
}

std::string HttpResponse::extToNature(const std::string &extension) {
    if (extension == ".html" || extension == ".txt") return "text/";
    else if (extension == ".jpg" || extension == ".png" || extension == ".gif") return "image/";
    else if (extension == ".json" || extension == ".xml" || extension == ".pdf") return "document/";
    else if (extension == ".zip") return "application/";
    else if (extension == ".mp3") return "audio/";
    else if (extension == ".mp4") return "video/";
    return "";
}

bool isDir(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
    //      holds info    is it a dir
}

void HttpResponse::handlePostRequest(const HttpRequest& request) {
    std::string path = request.getRequestBlock()->getRoot();
    std::string __contentType = request.getHeader("Content-Type");
    std::string __exten = ".bin"; // we need to save every content-type with its corresponding extension, ofc in a map
    std::string __folder = "";
    if (request.isChunkedRequest() == false){
        if (__contentType == "multipart/form-data") { // content-type is the thing that decides the file type, since the file name is not provided when we have raw/binary data transfer
            std::cout << MAGENTA << "Multi Form Data" << RESET << std::endl;
        }
        else { // chunked might be on/off
            if (this->postState == INIT_POST){ // first read
                this->postState = NEW_REQ_ENTRY;
                std::string ret = getConTypeExten(__contentType);
                std::cout << MAGENTA << "ret: <" << ret << ">" << RESET << std::endl;
                if (ret.empty() == false)
                    __exten = ret;
                ret = extToNature(__exten);
                if (ret.empty() == false)
                    __folder = ret;
                std::cout << RED << "Binary Data" << RESET << std::endl;
                this->fileName = randomizeFileName() + __exten;
                __folder = "uploads/" + __folder;
                std::cout << RED << "File name: " << this->fileName << RESET << std::endl;
                std::cout << YELLOW << "Content-Type: <" << __contentType << ">" << RESET << std::endl;
                if (isDir(__folder.c_str()) == true) {
                    this->fileName = __folder + this->fileName;
                    this->fd = open(this->fileName.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);// check for failure
                    write(this->fd, request.getBody().c_str(), request.getBody().size());
                }
                else {
                    std::cout << RED << "Folder do not exist!" << RESET << std::endl;
                    // throw an exception with an appropriate error page!!!!!!!!!!!!!
                }
                // !!!!!! what if the body is finished here ?
            }
            else {
                std::string buff = request.getReqEntry();
                write(this->fd, buff.c_str(), buff.size());
                if (this->postState == LAST_ENTRY) {
                    std::string connectState;
                    ConnectionState *state = ServerManager::getConnectionState(this->clientFd);
                    if (state->getIsKeepAlive())
                        connectState = "keep-alive";
                    else
                        connectState = "close";
                    const Server *server = request.getServer();
                    std::string serverName = server->getServerName();
                    int serverPort = server->getPort();
                    std::stringstream sS;
                    sS << serverName << ":" << serverPort << "/" << fileName;
                    this->version = request.getVersion();
                    this->statusCode = 201;
                    this->reasonPhrase = "Created";
                    this->headers["Content-Length"] = "11";
                    this->headers["Location"] = "/" + fileName;
                    this->body = "Lay3tik s7a";
                    std::string toStr = this->toString();
                    ServerManager::sendString(toStr, this->clientFd);
                }
            }
        }
    }
    else {
        std::cout << RED << "Chunked is not supported yet" << RESET << std::endl;
    }
}