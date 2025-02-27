#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cerrno>
#include <fstream>
#include <ios>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/types.h>
 #include <sys/stat.h>
#include <vector>
#include "../ResponseState/ResponseState.hpp"
#include <string>
#include <dirent.h>
#include <ctime>
#include <sys/socket.h>
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"

const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";
void HttpResponse::sendGetResponse(std::fstream &fileToGet, const std::string &filePath) const {
    std::vector<char> buffer(65536);  // 64KB buffer

    while (true) {
        fileToGet.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = fileToGet.gcount();
    
        if (bytesRead == 0)
            break;
    
        ssize_t totalSent = 0;
        while (totalSent < bytesRead) {
            ssize_t bytesSent = send(clientFd, buffer.data() + totalSent, bytesRead - totalSent, 0);
            if (bytesSent == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    std::streampos filePos = fileToGet.tellg();
                    filePos -= (bytesRead - totalSent);
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = this->clientFd;
                    ev.data.ptr = new ResponseState(filePath, filePos, this->clientFd, this->epollFd);
                    epoll_ctl(this->epollFd, EPOLL_CTL_MOD, this->clientFd, &ev);
                    return;
                } else {
                    perror("Error: ");
                    return ;
                }
            }
            totalSent += bytesSent;
        }
    }
}


void HttpResponse::handleAutoIndex(const HttpRequest& request) const {

    const std::string &path = request.getPath();
    const std::string &fullPath = request.getRequestBlock()->getRoot() + request.getPath();
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
                response += "<a href='" + path  + dName  + "/'>" + entry->d_name + "</a>";
            response += "<span class='size'>";
            std::stringstream ss;
            ss << statbuf.st_size;
            response += ss.str() + "</span>";
        }
        response += "</div>\n";
    }
    response += "</body></html>";

    std::stringstream headersSS;
    headersSS << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << response.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    send(this->clientFd, headersSS.str().c_str(), headersSS.str().size(), 0);
    send(this->clientFd, response.c_str(), response.size(), 0); // might need to improve this :D
    closedir(dir);
}

void HttpResponse::handleGetRequest(const HttpRequest& request) {
	std::string path = request.getRequestBlock()->getRoot() + request.getPath();
    if (path[path.size() - 1] == '/')
        path = path.substr(0, path.size() - 1);  // Remove last character properly
    

    struct stat filestat;
    if (stat(path.c_str(), &filestat) == -1)
        throw HttpErrorException(NOT_FOUND, request, "cant find file");
    if (filestat.st_mode & S_IFREG) {
        std::fstream fileToGet(path.c_str());
        if (fileToGet.fail() == true)
            throw HttpErrorException(NOT_FOUND, request, "cant find file");
    
        std::stringstream responseSs;
        responseSs << "HTTP/1.1 200 OK\r\n";
        responseSs << "Content-Type: " << request.getServer()->getMimeType(path.substr(path.find_last_of('.') + 1)) << "\r\n";
        responseSs << "Content-Length: " << filestat.st_size << "\r\n";
        responseSs << "\r\n";
        send(this->clientFd, responseSs.str().c_str(), responseSs.str().size(), 0);
    
        sendGetResponse(fileToGet, path);
    } else if (filestat.st_mode & S_IFDIR) {
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
            std::stringstream headersSS;
            headersSS << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: text/html\r\n"
                     << "Content-Length: " << indexstat.st_size << "\r\n"
                     << "Connection: close\r\n"
                     << "\r\n";
            send(this->clientFd, headersSS.str().c_str(), headersSS.str().size(), 0);
            std::fstream fileToGet(indexFilePath.c_str());
            sendGetResponse(fileToGet, indexFilePath);
        } else if (request.getRequestBlock()->getIsAutoIndexOn()) {
            handleAutoIndex(request);
        } else {
            throw HttpErrorException(NOT_FOUND, request, "Indexes not found and autoindex is off");
        }
    }
    
}
