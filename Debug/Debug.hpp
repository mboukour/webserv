#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "../Server/Server.hpp"
#include "../Http/HttpRequest/HttpRequest.hpp"
#define DEBUG 1
// Cout operator overloads
std::ostream& operator<<(std::ostream& outputStream, const Server& other);
std::ostream& operator<<(std::ostream& outputStream, const HttpRequest& request);


// Vector class printers
void printServers(std::vector<Server> servers); // Server.hpp

#endif