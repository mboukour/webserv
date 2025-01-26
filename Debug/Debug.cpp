#include "Debug.hpp"
#include <fstream>
#include <iostream>
#include <exception>
std::ostream& operator<<(std::ostream& outputStream, const Location& location) {
    std::string indent = "    ";  // 4 spaces for indentation

    outputStream << indent << "- Location: " << location.getLocationName() << "\n";
    outputStream << indent << "    Root Directory: " << location.getRoot() << "\n";
    outputStream << indent << "    Max Body Size: ";
    if (!location.getIsLimited())
        outputStream << "Unlimited\n";
    else
        outputStream << location.getMaxBodySize() << " bytes\n";
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
    outputStream << "Max Body Size: ";
    if (!other.getIsLimited())
        outputStream << "Unlimited\n";
    else
        outputStream << other.getMaxBodySize() << " bytes\n";
    outputStream << "GET Accepted: " << (other.getMethod(GET) ? "Yes" : "No") << "\n";
    outputStream << "POST Accepted: " << (other.getMethod(POST) ? "Yes" : "No") << "\n";
    outputStream << "DELETE Accepted: " << (other.getMethod(DELETE) ? "Yes" : "No") << "\n";
    outputStream << "Auto Index On: " << (other.getIsAutoIndexOn() ? "Yes" : "No") << "\n";
    outputStream << "Locations:\n";
    std::vector<Location>::const_iterator it;
    // for (it = other.locationsBegin(); it != other.locationsEnd(); ++it) {
    //     outputStream << *it;  // Calls the overloaded << for Location
    // }
    // if (other.locations.begin() == other.locations.end())
    //     throw std::logic_error("Hi");
    for (it = other.locationsCbegin(); it != other.locationsCend(); ++it) {
        outputStream << *it;  // Calls the overloaded << for Location
    }
    return outputStream;
}

std::ostream& operator<<(std::ostream& outputStream, const HttpRequest& request) {
    outputStream << "Method: " << request.getMethod() << "\n";
    outputStream << "Path: " << request.getPath() << "\n";
    outputStream << "Headers:\n";

    // change this by funcs that returns const iterators
    // std::map<std::string, std::string>::const_iterator it;
    // for (it = request.headers.begin(); it != request.headers.end(); ++it) {
    //     outputStream << "  " << it->first << ": " << it->second << "\n";
    // }

    outputStream << "Body Size: " << request.getBodySize() << "\n";
    outputStream << "Body: " << request.getBody() << "\n";
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