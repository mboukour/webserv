#include "Server.hpp"
#include <iostream> // to remove later

void Server::setPort(int port) {this->port = port;}

void Server::setServerName(const std::string &serverName) {this->serverName = serverName;}

int Server::getPort(void) const {return this->port;}

std::string Server::getServerName(void) const {return this->serverName;}

void Server::startServer(void) {std::cout << "This function currently does nothing...\n";}