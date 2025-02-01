#include "Server.hpp"
#include <iostream> // to remove later
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "../Debug/Debug.hpp"
#include <sstream>

Server::Server(): ABlock(), port(-1), host(""),  fdSocket(-1), serverName("") {}

Server::Server(const Server &other): ABlock(other), port(other.port), host(other.host) ,fdSocket(other.fdSocket), serverName(other.serverName), locations(other.locations) {}

void Server::setPort(int port) {this->port = port;}


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
    this->fdSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // listen to incoming clients
    if (this->fdSocket == -1)
    {
        errorStr = "Error: socket failed. Errno: ";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    fcntl(this->fdSocket, F_SETFL, O_NONBLOCK);
    // int opt = 1;
    // if (setsockopt(this->fdSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    // {
    //     errorStr = "Error: setsockopt failed. Errno: ";
    //     errorStr += strerror(errno);
    //     close(this->fdSocket);
    //     throw std::runtime_error(errorStr);
    // }
    if (bind(this->fdSocket, res->ai_addr, res->ai_addrlen) == -1)
    {
        errorStr = "Error: bind failed. Errno: ";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    if (listen(this->fdSocket, 10) == -1)
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