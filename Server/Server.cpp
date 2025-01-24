#include "Server.hpp"
#include <iostream> // to remove later
#include <cstdlib>

#include "../Debug/Debug.hpp"

Server::Server(): ABlock(), port(-1), fdSocket(-1), serverName("") {}

Server::Server(const Server &other): ABlock(other), port(other.port), fdSocket(other.fdSocket), serverName(other.serverName) {}

void Server::setPort(int port) {this->port = port;}

void Server::setServerName(const std::string &serverName) {this->serverName = serverName;}

int Server::getPort(void) const {return this->port;}

int Server::getFdSocket(void) const {return this->fdSocket;}

std::string Server::getServerName(void) const {return this->serverName;}

void Server::startServer(void) {
    struct sockaddr_in server_addr;
    std::string errorStr;
    this->fdSocket = socket(AF_INET, SOCK_STREAM, 0); // listen to incoming clients
    if (this->fdSocket == -1)
    {
        errorStr = "Error: socket failed. Errno: ";
        errorStr += strerror(errno);
        throw std::runtime_error(errorStr);
    }
    fcntl(this->fdSocket, F_SETFL, O_NONBLOCK);
    int opt = 1;
    if (setsockopt(this->fdSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        errorStr = "Error: setsockopt failed. Errno: ";
        errorStr += strerror(errno);
        close(this->fdSocket);
        throw std::runtime_error(errorStr);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(this->port);
    if (bind(this->fdSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
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

Server::~Server() {}