#include "Debug.hpp"
#include <fstream>
#include <iostream>

std::ostream& operator<<(std::ostream& outputStream, const Server& other) {
    outputStream << "Port: " << other.getPort() << "\n";
    outputStream << "Server Name: " << other.getServerName() << "\n";
    outputStream << "Root Directory: " << other.getRoot() << "\n";
    outputStream << "Max Body Size: " << other.getMaxBodySize() << " bytes\n";
    outputStream << "GET Accepted: " << (other.getIsGetAccepted() ? "Yes" : "No") << "\n";
    outputStream << "POST Accepted: " << (other.getIsPostAccepted() ? "Yes" : "No") << "\n";
    outputStream << "DELETE Accepted: " << (other.getIsDeleteAccepted() ? "Yes" : "No") << "\n";
    outputStream << "Auto Index On: " << (other.getIsAutoIndexOn() ? "Yes" : "No") << "\n";
    return outputStream;
}


void printServers(std::vector<Server> servers)
{
    int i = 0;
    for(std::vector<Server>::const_iterator it = servers.cbegin(); it != servers.cend(); it++, i++) {
        std::cout << "Server " << i << '\n'
                  << *it << "--------------------------------------------------------\n";
    }
}