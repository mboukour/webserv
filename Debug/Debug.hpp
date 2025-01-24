#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "../Server/Server.hpp"

// Cout operator overloads
std::ostream& operator<<(std::ostream& outputStream, const Server& other);



// Vector class printers
void printServers(std::vector<Server> servers); // Server.hpp

#endif