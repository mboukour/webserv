#include "Server.hpp"
#include <cstddef>
#include <iostream> // to remove later
#include <cstdlib>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../Debug/Debug.hpp"
#include "ServerManager/ServerManager.hpp"
#include <sstream>
#include <fstream>

Server::Server(): ABlock(), port(-1), host(""),  fdSocket(-1), serverName(""), isUp(false) {}

Server::Server(const Server &other): ABlock(other), port(other.port), host(other.host) ,fdSocket(other.fdSocket), serverName(other.serverName), locations(other.locations), mimeTypes(other.mimeTypes), isUp(other.isUp) {}

void Server::setPort(int port) {this->port = port;}

void Server::parseMimeTypeFile(const std::string &path) { // might add comment handling later
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos || path.substr(dotPos) != ".types")
        throw std::logic_error("Invalid mime_types file");
    std::fstream mimeTypeFile(path.c_str());
    if (!mimeTypeFile.is_open())
        throw std::logic_error("Invalid mime_types file");
    std::string line;
    while (std::getline(mimeTypeFile, line)) {
        std::stringstream ss(line);
        std::string extension;
        std::string type;
        ss >> extension >> type;
        if (ss.fail() || !ss.eof())
            throw std::logic_error("Invalid mime_types file");
        this->mimeTypes[extension] = type;
    }
}
// will return the actual type if it exists, if not it returns the default option: application/octet-stream
std::string Server::getMimeType(const std::string &extension) const {
    std::map<std::string, std::string> ::const_iterator it = this->mimeTypes.find(extension);
    if (it == this->mimeTypes.end())
        return "application/octet-stream";
    return it->second;
}

void Server::setServerName(const std::string &serverName) {this->serverName = serverName;}

int Server::getPort(void) const {return this->port;}

int Server::getFdSocket(void) const {return this->fdSocket;}



std::string Server::getServerName(void) const {return this->serverName;}

void Server::startServer(void) {
    struct addrinfo hints, *res;
    std::string errorStr;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::stringstream ss;
    ss << this->port;
    const char *hostCString = "0.0.0.0"; // any address
    if (this->host != "")
        hostCString = (char *)this->host.c_str();
    if (getaddrinfo(hostCString, ss.str().c_str(), &hints, &res) != 0)
    {
        errorStr = "Error: getaddrinfo failed. Errno: ";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    if (this->port < 1024)
        throw std::runtime_error("Ports below 1024 require root privileges");
    this->fdSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // listen to incoming clients
    if (this->fdSocket == -1)
    {
        errorStr = "Error: socket failed. Errno: ";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    this->isUp = true;
    fcntl(this->fdSocket, F_SETFL, O_NONBLOCK);
    int opt = 1;
    if (setsockopt(this->fdSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        errorStr = "Error: setsockopt failed. Errno: ";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    if (bind(this->fdSocket, res->ai_addr, res->ai_addrlen) == -1)
    {
        errorStr = "Error: bind failed. Errno: ";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    freeaddrinfo(res);
    if (listen(this->fdSocket, 128) == -1)
    {
        errorStr = "Error: listen failed. Errno: ";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    DEBUG && std::cout << "Server listening on port " << this->port << std::endl;
}


std::vector<Location>::iterator Server::locationsBegin(void) {
    return (this->locations.begin());
}

std::vector<Location>::iterator Server::locationsEnd(void) {
    return (this->locations.end());
}

std::vector<Location>::const_iterator Server::locationsCbegin(void) const {
    return (this->locations.begin());
}

std::vector<Location>::const_iterator Server::locationsCend(void) const {
    return (this->locations.end());
}

bool Server::isServerUp(void) const {
    return this->isUp;
}

void Server::addLocation(const Location &location) {
    this->locations.push_back(location);
}

void Server::setHost(const std::string &host) {
    this->host = host;
}

std::string Server::getHost(void) const {
    return this->host;
}

Server::~Server() {}
