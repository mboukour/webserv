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
    outputStream << indent << "    Index:\n";
    std::vector<std::string>::const_iterator iti;
    for (iti = location.indexCbegin(); iti != location.indexCend(); ++iti) {
        outputStream << indent << "        " << *iti << "\n";
    }
    outputStream << indent << "    GET Accepted: " << (location.getMethod(GET) ? "Yes" : "No") << "\n";
    outputStream << indent << "    POST Accepted: " << (location.getMethod(POST) ? "Yes" : "No") << "\n";
    outputStream << indent << "    DELETE Accepted: " << (location.getMethod(DELETE) ? "Yes" : "No") << "\n";
    outputStream << indent << "    Auto Index On: " << (location.getIsAutoIndexOn() ? "Yes" : "No") << "\n";
    outputStream << indent << "    Cgi:\n";
    std::map<std::string, std::string>::const_iterator itc;
    for (itc = location.cgisCbegin(); itc != location.cgisCend(); ++itc) {
        outputStream << indent << "        " << itc->first << ": " << itc->second << "\n";
    }
    outputStream << indent << "    Error Pages:\n";
    std::map<std::string, std::string>::const_iterator it;
    for (it = location.errorPagesCbegin(); it != location.errorPagesCend(); ++it) {
        outputStream << indent << "        " << it->first << ": " << it->second << "\n";
    }

    return outputStream;
}

std::ostream& operator<<(std::ostream& outputStream, const Server& other) {
    outputStream << "Host: ";
    std::string host = other.getHost();
    if (host.empty())
        std::cout << "Any\n";
    else
        std::cout << host << '\n';
    outputStream << "Port: " << other.getPort() << "\n";
    outputStream << "Server Name: " << other.getServerName() << "\n";
    outputStream << "Root Directory: " << other.getRoot() << "\n";
    outputStream << "Max Body Size: ";
    if (!other.getIsLimited())
        outputStream << "Unlimited\n";
    else
        outputStream << other.getMaxBodySize() << " bytes\n";
    outputStream << "Index:\n";
    std::vector<std::string>::const_iterator iti;
    for (iti = other.indexCbegin(); iti != other.indexCend(); ++iti) {
        outputStream << "  " << *iti << "\n";
    }
    outputStream << "GET Accepted: " << (other.getMethod(GET) ? "Yes" : "No") << "\n";
    outputStream << "POST Accepted: " << (other.getMethod(POST) ? "Yes" : "No") << "\n";
    outputStream << "DELETE Accepted: " << (other.getMethod(DELETE) ? "Yes" : "No") << "\n";
    outputStream << "Auto Index On: " << (other.getIsAutoIndexOn() ? "Yes" : "No") << "\n";
    outputStream << "Cgi:\n";
    std::map<std::string, std::string>::const_iterator itc;
    for (itc = other.cgisCbegin(); itc != other.cgisCend(); ++itc) {
        outputStream << "  " << itc->first << ": " << itc->second << "\n";
    }
    outputStream << "Error Pages:\n";
    std::map<std::string, std::string>::const_iterator ite;
    for (ite = other.errorPagesCbegin(); ite != other.errorPagesCend(); ++ite) {
        outputStream << "  " << ite->first << ": " << ite->second << "\n";
    }

    // outputStream << "Redirect:\n";
    // outputStream << other.get
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
    outputStream << "Method: " << request.getMethod() << " ";
    outputStream << "Path: " << request.getPath() << " ";
    outputStream << "Query string: " << request.getQueryString() << "\n";
    outputStream << "Headers:\n";


    // change this by funcs that returns const iterators
    // std::map<std::string, std::string>::const_iterator it;
    // for (it = request.headers.begin(); it != request.headers.end(); ++it) {
    //     outputStream << "  " << it->first << ": " << it->second << "\n";
    // }
    request.printHeaders();
    // outputStream << "Body Size: " << request.getBodySize() << "\n";
    // outputStream << "Body: " << request.getBody() << "\n";
    // std::cout << "Cookies: \n";
    // std::map<std::string, std::string> cookies = request.getCookie();
    // for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); it++) {
    //     std::cout << it->first << ": " << it->second << '\n';
    // }
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

