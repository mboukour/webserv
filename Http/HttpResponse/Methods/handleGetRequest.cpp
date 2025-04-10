#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cerrno>
#include <fstream>
#include <ios>
#include <netinet/in.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/types.h>
 #include <sys/stat.h>
#include <vector>
#include "../../../ClientState/ClientState.hpp"
#include <string>
#include <dirent.h>
#include <ctime>
#include <sys/socket.h>
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "../../../Server/ServerManager/ServerManager.hpp"

const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";


void HttpResponse::handleAutoIndex(const HttpRequest& request) {

    const std::string &path = request.getPath();
    const std::string &fullPath = request.getRequestBlock()->getRoot() + request.getPath();
    std::cout << YELLOW << fullPath << RESET << '\n';
    DIR *dir = opendir(fullPath.c_str());
    if (!dir) 
        throw HttpErrorException(NOT_FOUND, request, "Can't open directory");
    std::string response;
    response += "<!DOCTYPE html>\n<html>\n<title>Index of ";
    response += path;
    response += "</title>\n<style>\nbody {\nfont-family: -apple-system, sans-serif;\nmax-width: 1000px;\nmargin: 20px auto;\npadding: 0 20px;\n}\n.item {\ndisplay: grid;\ngrid-template-columns: 20px 1fr 100px 160px;\npadding: 8px;\nborder-bottom: 1px solid #eee;\ntext-decoration: none;\ncolor: inherit;\n}\n.item:hover { background: #f5f5f5; }\n.size { text-align: right; color: #666; }\n.date { text-align: right; color: #666; }\n</style></head><body><h1>Index of ";
    response += path;
    response += "</h1>";
    struct dirent *entry;
    while((entry = readdir(dir))) {
        std::string dName(entry->d_name);
        if (dName == "." || dName == "..")
            continue;
        struct stat statbuf;
        std::string entryPath = fullPath + "/" + dName;
        if (stat(entryPath.c_str(), &statbuf) == -1)
            continue;
        response += "<div class='item'>";
        if (S_ISDIR(statbuf.st_mode)) {
            response += "<svg viewBox='0 0 24 24' width='16'>"
                       "<path d='M10 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2h-8l-2-2z'/>"
                       "</svg>";
            if (path[path.size() - 1 ] != '/')
                response += "<a href='" + path + '/' + dName  + "/'>" + entry->d_name + "</a>";
            else
                response += "<a href='" + path  + dName  + "/'>" + entry->d_name + "</a>";
            response += "<span class='size'>-</span>";
        } else {
            response += "<svg viewBox='0 0 24 24' width='16'>"
                       "<path d='M6 2c-1.1 0-1.99.9-1.99 2L4 20c0 1.1.89 2 1.99 2H18c1.1 0 2-.9 2-2V8l-6-6H6zm7 7V3.5L18.5 9H13z'/>"
                       "</svg>";
            if (path[path.size() - 1 ] != '/')
                response += "<a href='" + path + '/' + dName  + "/'>" + entry->d_name + "</a>";
            else
                response += "<a href='" + path  + dName  + "'>" + entry->d_name + "</a>";
            response += "<span class='size'>";
            std::stringstream ss;
            ss << statbuf.st_size;
            response += ss.str() + "</span>";
        }
        response += "</div>\n";
    }
    response += "</body></html>";

    this->version = request.getVersion();
	this->statusCode = 200;
	this->reasonPhrase = HttpErrorException::getReasonPhrase(statusCode);
    this->headers["Content-Type"] = "text/html";
    std::stringstream ss;
    ss << response.size();
    this->headers["Content-Length"] = ss.str();
    ClientState *state = ServerManager::getClientState(this->clientFd);
    if (state->getIsKeepAlive()) {
        this->headers["Connection"] = "keep-alive";
        this->headers["Keep-Alive"] = "timeout=10, max=1000";
    } else
        this->headers["Connection"] = "close";

    ServerManager::sendString(toString(), this->clientFd);
    ServerManager::sendString(response, this->clientFd);
    closedir(dir);
}

void HttpResponse::handleGetRequest(const HttpRequest& request) {
	std::string path = request.getRequestBlock()->getRoot() + request.getPath();

    struct stat filestat;
    if (stat(path.c_str(), &filestat) == -1) 
        throw HttpErrorException(NOT_FOUND, request, "cant find file");
    if (filestat.st_mode & S_IFREG) {
        std::fstream fileToGet(path.c_str());
        if (fileToGet.fail() == true)
            throw HttpErrorException(NOT_FOUND, request, "cant find file");
        HttpResponse response("HTTP/1.1", 200, "OK", "");
        response.setHeader("Content-Type", request.getServer()->getMimeType(path.substr(path.find_last_of('.') + 1)));
        std::stringstream ss;
        ss << filestat.st_size;
        response.setHeader("Content-Length", ss.str());
        ServerManager::sendString(response.toString(), this->clientFd);
        ServerManager::sendFile(path, this->clientFd);
    } else if (filestat.st_mode & S_IFDIR) {
        if (path[path.size() - 1] != '/') {
            HttpResponse redirectResponse(request.getVersion(), 301, HttpErrorException::getReasonPhrase(301), "");
            redirectResponse.setHeader("Location", request.getPath() + "/");
            ServerManager::sendString(redirectResponse.toString(), this->clientFd);
            return ;
        }
        std::vector<std::string> indexes = request.getRequestBlock()->getIndexes();
        struct stat indexstat;
        bool foundIndex = false;
        std::string indexFilePath;
        for (std::vector<std::string>::const_iterator it = indexes.begin(); it != indexes.end(); it++) {
            indexFilePath = path + "/" + *it;
            if (stat(indexFilePath.c_str(), &indexstat) != -1) {
                foundIndex = true;
                break;
            }
        }
        if (foundIndex) {
            if (isCgiFile(indexFilePath, request)) {
                throw std::logic_error("Fix autoindex cgi");
                return ;
            }
            std::stringstream cl;
            cl << indexstat.st_size;
            this->version = request.getVersion();
            this->statusCode = 200;
            this->reasonPhrase = "OK";
            this->headers["Content-Type"] = "text/html";
            this->headers["Content-Length"] = cl.str();
        	ClientState *state = ServerManager::getClientState(this->clientFd);
            if (state->getIsKeepAlive()) {
                this->headers["Keep-Alive"] = "timeout=10";
            }
            this->body.clear();
            ServerManager::sendString(toString(), this->clientFd);
            ServerManager::sendFile(indexFilePath, this->clientFd);
        } else if (request.getRequestBlock()->getIsAutoIndexOn()) {
            handleAutoIndex(request);
        } else {
            throw HttpErrorException(FORBIDDEN, request, "Indexes not found and autoindex is off");
        }
    }    
}
