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
#include "../ResponseState/ResponseState.hpp"
const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";
#include <string>
#include <sys/socket.h>
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"

void HttpResponse::sendGetResponse(std::fstream &fileToGet, const std::string &filePath) const {
    std::vector<char> buffer(65536);  // 64KB buffer

    while (true) {
        fileToGet.read(buffer.data(), buffer.size());
        std::streamsize bytesRead = fileToGet.gcount();
    
        if (bytesRead == 0)
            break;  // EOF
    
        // Send the data
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

void HttpResponse::handleGetRequest(const HttpRequest& request) {
	std::string path = request.getRequestBlock()->getRoot() + request.getPath();

    struct stat filestat;
    if (stat(path.c_str(), &filestat) == -1)
        throw HttpErrorException(NOT_FOUND, request, "cant find file");
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
}
