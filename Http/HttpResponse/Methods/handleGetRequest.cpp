#include "../HttpResponse.hpp"
#include "../../HttpRequest/HttpRequest.hpp"
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

const std::string YELLOW = "\033[33m";
const std::string RESET = "\033[0m";
#include <string>
#include <sys/socket.h>
#include "../../../Exceptions/HttpErrorException/HttpErrorException.hpp"

std::string const generateContent(const HttpRequest &request, HttpResponse &response, std::string const path){
    std::string content;
    (void)response;
    (void)request;
    std::fstream fileSs(path.c_str());
    if (fileSs.fail() == true)
        throw HttpErrorException(request.getVersion(), NOT_FOUND, "Not Found", "cant find file", request.getRequestBlock()->getErrorPageHtml(NOT_FOUND));
    std::string line;
    while (std::getline(fileSs, line))
        content += line + "\n";
    return content;
}

// std::string generateHeader(){

// }


void HttpResponse::handleGetRequest(const HttpRequest& request) {
	std::string path = request.getRequestBlock()->getRoot() + request.getPath();
    std::cout << YELLOW << "path: [" << path << "]" << RESET << std::endl;
    std::string const content = generateContent(request, *this, path);
	std::stringstream responseSs;
    std::cout << YELLOW << "path: [" << path << "]" << RESET << std::endl;
	responseSs << "HTTP/1.1 200 OK\r\n";
	responseSs << "Content-Type: text/html\r\n";
	responseSs << "Connection: close\r\n";
	responseSs << "Content-Length: " << content.length() << "\r\n";
	responseSs << "\r\n";
	responseSs << content;
    std::string response = responseSs.str();
    std::cout << YELLOW << response << RESET << std::endl;
	send(this->clientFd, response.c_str(), response.size(), 0);
	// throw HttpErrorException(request.getVersion(), NOT_IMPLEMENTED, "Not Implemented", "no code yet ;)", request.getRequestBlock()->getErrorPageHtml(NOT_IMPLEMENTED));
}
