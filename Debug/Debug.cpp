#include "Debug.hpp"
#include <fstream>
#include <iostream>

std::ostream& operator<<(std::ostream& outputStream, const Location& location) {
    std::string indent = "    ";  // 4 spaces for indentation

    outputStream << indent << "- Location: " << location.getLocationName() << "\n";
    outputStream << indent << "    Root Directory: " << location.getRoot() << "\n";
    outputStream << indent << "    Max Body Size: " << location.getMaxBodySize() << " bytes\n";
    outputStream << indent << "    GET Accepted: " << (location.getMethod(GET) ? "Yes" : "No") << "\n";
    outputStream << indent << "    POST Accepted: " << (location.getMethod(POST) ? "Yes" : "No") << "\n";
    outputStream << indent << "    DELETE Accepted: " << (location.getMethod(DELETE) ? "Yes" : "No") << "\n";
    outputStream << indent << "    Auto Index On: " << (location.getIsAutoIndexOn() ? "Yes" : "No") << "\n";

    return outputStream;
}

std::ostream& operator<<(std::ostream& outputStream, const Server& other) {
    outputStream << "Port: " << other.getPort() << "\n";
    outputStream << "Server Name: " << other.getServerName() << "\n";
    outputStream << "Root Directory: " << other.getRoot() << "\n";
    outputStream << "Max Body Size: " << other.getMaxBodySize() << " bytes\n";
    outputStream << "GET Accepted: " << (other.getMethod(GET) ? "Yes" : "No") << "\n";
    outputStream << "POST Accepted: " << (other.getMethod(POST) ? "Yes" : "No") << "\n";
    outputStream << "DELETE Accepted: " << (other.getMethod(DELETE) ? "Yes" : "No") << "\n";
    outputStream << "Auto Index On: " << (other.getIsAutoIndexOn() ? "Yes" : "No") << "\n";
    outputStream << "Locations:\n";
    if (other.locations.empty())
        outputStream << "No locations found.\n";
    else
    {
        std::vector<Location>::const_iterator it;
        for (it = other.locations.begin(); it != other.locations.end(); ++it) {
            outputStream << *it;  // Calls the overloaded << for Location
        }
    }
    return outputStream;
}


void printServers(std::vector<Server> servers)
{
    int i = 0;
    for(std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++, i++) {
        std::cout << "Server " << i << '\n'
                  << *it << "--------------------------------------------------------\n";
    }
}